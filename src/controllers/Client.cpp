#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <controllers/Client.hpp>
#include <models/MessageHeader.hpp>
#include <models/RequestSetChunkSize.hpp>
#include <utils/Chunk.hpp>
#include <utils/Md5.hpp>
#include <utils/Volume.hpp>

namespace VolSync
{

Client::Child::Child(
    int readDescriptor,
    int writeDescriptor):
    readDescriptor(readDescriptor),
    writeDescriptor(writeDescriptor)
{
}

Client::Client(
    const std::string& source,
    const std::string& target,
    const CommandLineArguments& commandLineArguments):
    m_sourceVolume(source),
    m_commandLineArguments(commandLineArguments)
{
    auto colonPosition = target.find(':');
    if (colonPosition == target.npos)
    {
        throw std::invalid_argument("invalid target format");
    }

    m_remoteHost = target.substr(0, colonPosition);
    m_targetVolume = target.substr(colonPosition + 1);

    if (m_remoteHost.empty())
    {
        throw std::invalid_argument("no remote host given");
    }
    if (m_targetVolume.empty())
    {
        throw std::invalid_argument("no target volume given");
    }
    if (m_targetVolume[0] != '/')
    {
        throw std::invalid_argument("target volume must be an absolute path");
    }
}

Client::~Client()
{
}

void Client::run(void)
{
    auto child = startRemoteServer(
        m_commandLineArguments.sshCommand,
        m_remoteHost,
        m_commandLineArguments.remotePathToExecutable,
        m_targetVolume);

    auto clientVersion = ResponseVersion();
    auto serverVersion = getServerVersion(child);

    if (std::string(serverVersion.packageName) != std::string(clientVersion.packageName) ||
        serverVersion.protocolVersion != clientVersion.protocolVersion)
    {
        std::cerr << "ERROR: Detected version mismatch!" << std::endl;
        std::cerr << "Client: " << clientVersion << std::endl;
        std::cerr << "Server: " << serverVersion << std::endl;
        requestAbort(child);

        throw std::runtime_error("version mismatch detected");
    }
    else
    {
        std::cout << "Detected version: " << serverVersion << std::endl;
    }

    auto sourceVolumeInformation = Volume::getInformation(m_sourceVolume);
    auto targetVolumeInformation = getTargetVolumeInformation(child);

    if (targetVolumeInformation.size < sourceVolumeInformation.size)
    {
        throw std::runtime_error(
            "target volume is smaller than the source (" +
                std::to_string(targetVolumeInformation.size) + " < " +
                std::to_string(sourceVolumeInformation.size) + ")");
    }

    openVolume();
    requestOpenVolume(child);

    m_chunkSize = determineBestChunkSize(sourceVolumeInformation.size);
    std::cout << "Best chunk size: " << m_chunkSize << std::endl;
    setChunkSize(child, m_chunkSize);

    auto chunkCount = sourceVolumeInformation.size / m_chunkSize;
    uint64_t lastStatusChunkIndex = 0;
    uint64_t updatedChunks = 0;
    timespec referenceTime;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &referenceTime);
    auto startTime = referenceTime;

    for (m_chunkIndex = 0; m_chunkIndex < chunkCount; ++m_chunkIndex)
    {
        setChunkIndex(child, m_chunkIndex);

        seekToChunkIndex(m_chunkIndex);
        auto clientChunkHash = Md5::calculateChunkHash(m_volumeDescriptor, m_chunkSize);
        auto serverChunkHash = getChunkHash(child);
        if (clientChunkHash != serverChunkHash)
        {
            seekToChunkIndex(m_chunkIndex);
            auto chunk = Chunk::read(m_volumeDescriptor, m_chunkSize);
            writeChunkToTarget(child, chunk);

            if (m_commandLineArguments.verifyAfterWrite)
            {
                seekToChunkIndex(m_chunkIndex);
                clientChunkHash = Md5::calculateChunkHash(m_volumeDescriptor, m_chunkSize);
                serverChunkHash = getChunkHash(child);
                if (clientChunkHash != serverChunkHash)
                {
                    throw std::runtime_error("chunk validation failed");
                }
            }
            ++updatedChunks;
        }

        if (m_commandLineArguments.statusInterval > 0)
        {
            timespec now;
            clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
            if (now.tv_sec >= referenceTime.tv_sec + m_commandLineArguments.statusInterval)
            {
                char buffer[32];
                auto runtime = now.tv_sec - startTime.tv_sec;

                snprintf(
                    buffer,
                    sizeof(buffer),
                    "%02d:%02d:%02d",
                    (int) runtime / 3600,
                    (int) ((runtime / 60) % 60),
                    (int) runtime % 60);
                std::cout << "[" << buffer << "] ";

                snprintf(buffer, sizeof(buffer), "%.2f", m_chunkIndex * 100.0 / chunkCount);
                std::cout <<
                    "Progress: " << buffer << "% "
                    "(" << m_chunkIndex << "/" << chunkCount << ", " << updatedChunks << " updated, ";

                auto transmittedChunks = m_chunkIndex - lastStatusChunkIndex;
                if (transmittedChunks == 0)
                {
                    std::cout << "-- stalled --";
                }
                else
                {
                    auto transmissionSpeed = (transmittedChunks * m_chunkSize) / (now.tv_sec - referenceTime.tv_sec);
                    if (transmissionSpeed >= (4 * 1024 * 1024))
                    {
                        snprintf(buffer, sizeof(buffer), "%.2f", transmissionSpeed / (1024.0 * 1024.0));
                        std::cout << buffer << " MiB/s";
                    }
                    else if (transmissionSpeed >= (4 * 1024))
                    {
                        snprintf(buffer, sizeof(buffer), "%.2f", transmissionSpeed / 1024.0);
                        std::cout << buffer << " KiB/s";
                    }
                    else
                    {
                        std::cout << transmissionSpeed << " Byte/s";
                    }
                }
                std::cout << ")" << std::endl;

                lastStatusChunkIndex = m_chunkIndex;
                referenceTime = now;
            }
        }
    }

    closeVolume();
    requestCloseVolume(child);
    requestAbort(child);
}

void Client::openVolume(void)
{
    closeVolume();

    auto descriptor = open(m_sourceVolume.c_str(), O_RDONLY);
    if (descriptor < 0)
    {
        throw std::runtime_error(
            "could not open volume \"" + m_sourceVolume + "\" for reading (" + std::string(strerror(errno)) + ")");
    }

    m_volumeDescriptor = descriptor;
}

ResponseVersion Client::getServerVersion(const Child& child)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_VERSION, ByteArray(),
        MessageType::RESPONSE_VERSION, responsePayload);

    return ResponseVersion::fromByteArray(responsePayload);
}

ResponseVolumeInformation Client::getTargetVolumeInformation(const Child& child)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_VOLUME_INFORMATION, ByteArray(),
        MessageType::RESPONSE_VOLUME_INFORMATION, responsePayload);

    return ResponseVolumeInformation::fromByteArray(responsePayload);
}

void Client::requestOpenVolume(const Child& child)
{
    doSimpleRequest(child, MessageType::REQUEST_OPEN_VOLUME, MessageType::RESPONSE_OPEN_VOLUME);
}

void Client::requestCloseVolume(const Child& child)
{
    doSimpleRequest(child, MessageType::REQUEST_CLOSE_VOLUME, MessageType::RESPONSE_CLOSE_VOLUME);
}

void Client::setChunkSize(const Child& child, uint64_t chunkSize)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_SET_CHUNK_SIZE, RequestSetChunkSize(chunkSize).toByteArray(),
        MessageType::RESPONSE_SET_CHUNK_SIZE, responsePayload);
}

void Client::setChunkIndex(const Child& child, uint64_t chunkIndex)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_SET_CHUNK_INDEX, RequestSetChunkSize(chunkIndex).toByteArray(),
        MessageType::RESPONSE_SET_CHUNK_INDEX, responsePayload);
}

ResponseGetChunkHash Client::getChunkHash(const Child& child)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_GET_CHUNK_HASH, ByteArray(),
        MessageType::RESPONSE_GET_CHUNK_HASH, responsePayload);

    return ResponseGetChunkHash::fromByteArray(responsePayload);
}

void Client::requestAbort(const Child& child)
{
    doSimpleRequest(child, MessageType::REQUEST_ABORT, MessageType::RESPONSE_ABORT);
}

void Client::writeChunkToTarget(const Child& child, const ByteArray& chunk)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_WRITE_CHUNK_TO_TARGET, chunk,
        MessageType::RESPONSE_WRITE_CHUNK_TO_TARGET, responsePayload);
}

void Client::doSimpleRequest(const Child& child, MessageType request, MessageType expectedResponse)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        request, ByteArray(),
        expectedResponse, responsePayload);
}

void Client::communicateWithServer(
    const Child& child,
    MessageType request,
    const ByteArray& requestPayload,
    MessageType expectedResponse,
    ByteArray& responsePayload)
{
    auto messageHeader = MessageHeader(request, requestPayload.size());
    // std::cout << "CLIENT TX >> " << messageHeader << std::endl;

    auto bytesWritten = write(child.writeDescriptor, &messageHeader, sizeof(messageHeader));
    if (bytesWritten != (ssize_t) sizeof(messageHeader))
    {
        throw std::runtime_error(
            "unexpected number of message header bytes written "
            "(" + std::to_string(bytesWritten) + " != " + std::to_string(sizeof(messageHeader)) + ", " +
            std::string(strerror(errno)) + ")");
    }

    bytesWritten = write(child.writeDescriptor, requestPayload.c_str(), requestPayload.size());
    if (bytesWritten != (ssize_t) requestPayload.size())
    {
        throw std::runtime_error(
            "unexpected number of payload bytes written "
            "(" + std::to_string(bytesWritten) + " != " + std::to_string(requestPayload.size()) + ", " +
            std::string(strerror(errno)) + ")");
    }

    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(child.readDescriptor, &descriptors);
    timeval timeout { readTimeoutSecondsDefault, 0 };

    auto selectResult = select(child.readDescriptor + 1, &descriptors, nullptr, nullptr, &timeout);
    if (selectResult > 0)
    {
        if (!FD_ISSET(child.readDescriptor, &descriptors))
        {
            throw std::runtime_error("unexpected result from select()");
        }

        auto bytesRead = read(child.readDescriptor, &messageHeader, sizeof(messageHeader));
        if (bytesRead <= 0)
        {
            throw std::runtime_error("server closed connected unexpectedly");
        }
        if (bytesRead != (ssize_t) sizeof(messageHeader))
        {
            throw std::runtime_error(
                "unexpected number of message header bytes read "
                "(" + std::to_string(bytesRead) + " != " + std::to_string(sizeof(messageHeader)) + ", " +
                std::string(strerror(errno)) + ")");
        }
        // std::cout << "CLIENT RX << " << messageHeader << std::endl;

        if (messageHeader.payloadLength > receiveBufferMax)
        {
            throw std::runtime_error(
                "actual payload length exceeds the allowed receive buffer "
                "(" + std::to_string(messageHeader.payloadLength) + " > " + std::to_string(receiveBufferMax) + ", " +
                std::string(strerror(errno)) + ")");
        }

        responsePayload.clear();
        if (messageHeader.payloadLength > 0)
        {
            uint8_t buffer[messageHeader.payloadLength];
            bytesRead = read(child.readDescriptor, buffer, messageHeader.payloadLength);

            if (bytesRead <= 0 || bytesRead != (ssize_t) messageHeader.payloadLength)
            {
                throw std::runtime_error(
                    "unexpected number of payload bytes read "
                    "(" + std::to_string(bytesRead) + " != " + std::to_string(messageHeader.payloadLength) + ", " +
                    std::string(strerror(errno)) + ")");
            }

            responsePayload = ByteArray(buffer, bytesRead);
        }

        if (messageHeader.messageType != static_cast<uint32_t>(expectedResponse))
        {
            throw std::runtime_error(
                "unexpected response type received "
                "(" + std::to_string(messageHeader.messageType) + " != " +
                std::to_string(static_cast<uint32_t>(expectedResponse)) + ")");
        }
    }
    else if (selectResult == 0)
    {
        throw std::runtime_error("select() timed out");
    }
    else if (selectResult < 0)
    {
        throw std::runtime_error("select() returned an error (" + std::string(strerror(errno)) + ")");
    }
}

Client::Child Client::startRemoteServer(
    const std::string& sshCommand,
    const std::string& remoteHost,
    const std::string& remotePathToExecutable,
    const std::string& targetVolume)
{
    if (sshCommand.empty())
    {
        throw std::invalid_argument("empty SSH command not allowed");
    }

    auto arguments = splitCommand(sshCommand);
    arguments.push_back(remoteHost);

    if (remotePathToExecutable.empty())
    {
        arguments.push_back(remoteCommandDefault);
    }
    else
    {
        arguments.push_back(remotePathToExecutable + remoteCommandDefault);
    }

    arguments.push_back("-s");
    arguments.push_back(targetVolume);

    // std::cout << "arguments=" << arguments << std::endl;

    return startRemoteServer(arguments);
}

Client::Child Client::startRemoteServer(const ProcessArguments& arguments)
{
    int toChildPipe[2];
    int fromChildPipe[2];

    if (pipe(toChildPipe) < 0 || pipe(fromChildPipe) < 0)
    {
        throw std::runtime_error("could not set up pipes");
    }

    auto pid = fork();

    if (pid < 0)
    {
        throw std::runtime_error("could not fork (" + std::string(strerror(errno)) + ")");
    }

    if (pid == 0)
    {
        if (dup2(toChildPipe[0], STDIN_FILENO) < 0 ||
            close(toChildPipe[1]) < 0 ||
            close(fromChildPipe[0]) < 0 ||
            dup2(fromChildPipe[1], STDOUT_FILENO) < 0)
        {
            throw std::runtime_error("could not duplicate/close the pipes (" + std::string(strerror(errno)) + ")");
        }

        if (toChildPipe[0] != STDIN_FILENO)
        {
            close(toChildPipe[0]);
        }
        if (fromChildPipe[1] != STDOUT_FILENO)
        {
            close(fromChildPipe[1]);
        }

        const char* argumentList[arguments.size() + 1];
        unsigned int i = 0;
        for (; i < arguments.size(); ++i)
        {
            argumentList[i] = arguments[i].c_str();
        }
        argumentList[i] = nullptr;


        if (execvp(argumentList[0], (char * const *) argumentList))
        {
            throw std::runtime_error("could not run command (" + std::string(strerror(errno)) + ")");
        }
    }

    close(toChildPipe[0]);
    close(fromChildPipe[1]);

    return Child(fromChildPipe[0], toChildPipe[1]);
}

Client::ProcessArguments Client::splitCommand(const std::string& command)
{
    ProcessArguments arguments;
    std::unique_ptr<char> temp { strdup(command.c_str()) };

    auto token = strtok(temp.get(), " ");
    while (token)
    {
        if (strlen(token) > 0)
        {
           arguments.push_back(token);
        }

        token = strtok(nullptr, " ");
    }

    return arguments;
}

uint64_t Client::determineBestChunkSize(uint64_t volumeSize)
{
    if ((volumeSize & (chunkSizeMin - 1)) != 0)
    {
        throw std::runtime_error(
            "volume size is not a multiple of the minimum chunk size (" + std::to_string(chunkSizeMin) + ")");
    }

    auto chunkSize = chunkSizeMin;
    while ((volumeSize & ((chunkSize << 1UL) - 1)) == 0 && ((chunkSize << 1UL) <= chunkSizeMax))
    {
        chunkSize <<= 1UL;
    }

    return chunkSize;
}

std::ostream& operator<<(std::ostream& stream, const Client::ProcessArguments& arguments)
{
    stream << "(";
    for (auto& entry: arguments)
    {
        stream << entry << ",";
    }
    stream << ")";

    return stream;
}

} /* namespace VolSync */
