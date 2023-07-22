#include <unistd.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <controllers/Client.hpp>
#include <models/MessageHeader.hpp>
#include <models/RequestSetChunkSize.hpp>
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

    auto chunkSize = determineBestChunkSize(sourceVolumeInformation.size);
    std::cout << "Best chunk size: " << chunkSize << std::endl;
    setChunkSize(child, chunkSize);

    auto chunkCount = sourceVolumeInformation.size / chunkSize;
    for (uint64_t chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex)
    {
        setChunkIndex(child, chunkIndex);
    }

    requestAbort(child);
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

void Client::requestAbort(const Child& child)
{
    ByteArray responsePayload;
    communicateWithServer(
        child,
        MessageType::REQUEST_ABORT, ByteArray(),
        MessageType::RESPONSE_ABORT, responsePayload);
}

void Client::communicateWithServer(
    const Child& child,
    MessageType request,
    const ByteArray& requestPayload,
    MessageType expectedResponse,
    ByteArray& responsePayload)
{
    auto messageHeader = MessageHeader(request, requestPayload.size());

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
            auto buffer = std::make_unique<uint8_t>((int) messageHeader.payloadLength);
            bytesRead = read(child.readDescriptor, buffer.get(), messageHeader.payloadLength);

            if (bytesRead <= 0 || bytesRead != (ssize_t) messageHeader.payloadLength)
            {
                throw std::runtime_error(
                    "unexpected number of payload bytes read "
                    "(" + std::to_string(bytesRead) + " != " + std::to_string(messageHeader.payloadLength) + ", " +
                    std::string(strerror(errno)) + ")");
            }

            responsePayload = ByteArray(buffer.get(), bytesRead);
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

    std::cout << "arguments=" << arguments << std::endl;

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
