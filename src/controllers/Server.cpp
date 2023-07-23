#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <controllers/Server.hpp>
#include <models/MessageHeader.hpp>
#include <models/RequestSetChunkIndex.hpp>
#include <models/RequestSetChunkSize.hpp>
#include <models/ResponseVersion.hpp>
#include <utils/ByteArray.hpp>
#include <utils/Chunk.hpp>
#include <utils/Md5.hpp>
#include <utils/Volume.hpp>

namespace VolSync
{

Server::Server(
    const std::string& targetVolume):
    m_targetVolume(targetVolume)
{
}

Server::~Server()
{
}

void Server::run(void)
{
    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(STDIN_FILENO, &descriptors);
    timeval timeout { readTimeoutSecondsDefault, 0 };

    int selectResult;
    bool quitLoop = false;
    while ((selectResult = select(STDIN_FILENO + 1, &descriptors, nullptr, nullptr, &timeout)) > 0 && !quitLoop)
    {
        if (!FD_ISSET(STDIN_FILENO, &descriptors))
        {
            throw std::runtime_error("unexpected result from select()");
        }

        MessageHeader messageHeader;
        auto bytesRead = read(STDIN_FILENO, &messageHeader, sizeof(messageHeader));
        if (bytesRead <= 0)
        {
            throw std::runtime_error("client closed connected unexpectedly");
        }
        if (bytesRead != (ssize_t) sizeof(messageHeader))
        {
            throw std::runtime_error(
                "unexpected number of message header bytes read "
                "(" + std::to_string(bytesRead) + " != " + std::to_string(sizeof(messageHeader)) + ", " +
                std::string(strerror(errno)) + ")");
        }
        // std::cerr << "SERVER RX << " << messageHeader << std::endl;

        if (messageHeader.payloadLength > receiveBufferMax)
        {
            throw std::runtime_error(
                "actual payload length exceeds the allowed receive buffer "
                "(" + std::to_string(messageHeader.payloadLength) + " > " + std::to_string(receiveBufferMax) + ")");
        }

        ByteArray payload;
        if (messageHeader.payloadLength > 0)
        {
            uint8_t buffer[messageHeader.payloadLength];
            auto payloadLength = messageHeader.payloadLength;
            ssize_t totalBytesRead = 0;

            while (totalBytesRead < payloadLength &&
                (bytesRead = read(STDIN_FILENO, buffer + totalBytesRead, payloadLength - totalBytesRead)) > 0)
            {
                totalBytesRead += bytesRead;
            }

            if (bytesRead <= 0 || totalBytesRead != payloadLength)
            {
                throw std::runtime_error(
                    "unexpected number of payload bytes read "
                    "(" + std::to_string(totalBytesRead) + " != " + std::to_string(payloadLength) + ", " +
                    std::string(strerror(errno)) + ")");
            }

            payload = ByteArray(buffer, totalBytesRead);
        }

        switch (static_cast<MessageType>(messageHeader.messageType))
        {
            case MessageType::REQUEST_ABORT:
                std::cerr << "SERVER: received REQUEST_ABORT, shutting down server" << std::endl;
                respondToClient(MessageType::RESPONSE_ABORT);
                quitLoop = true;
                break;
            case MessageType::REQUEST_VERSION:
                respondToClient(MessageType::RESPONSE_VERSION, ResponseVersion().toByteArray());
                break;
            case MessageType::REQUEST_VOLUME_INFORMATION:
                respondToClient(
                    MessageType::RESPONSE_VOLUME_INFORMATION,
                    Volume::getInformation(m_targetVolume).toByteArray());
                break;
            case MessageType::REQUEST_OPEN_VOLUME:
                openVolume();
                respondToClient(
                    MessageType::RESPONSE_OPEN_VOLUME);
                break;
            case MessageType::REQUEST_CLOSE_VOLUME:
                closeVolume();
                respondToClient(
                    MessageType::RESPONSE_CLOSE_VOLUME);
                break;
            case MessageType::REQUEST_SET_CHUNK_SIZE:
                m_chunkSize = RequestSetChunkSize::fromByteArray(payload).size;
                respondToClient(MessageType::RESPONSE_SET_CHUNK_SIZE);
                break;
            case MessageType::REQUEST_SET_CHUNK_INDEX:
                m_chunkIndex = RequestSetChunkIndex::fromByteArray(payload).chunkIndex;
                respondToClient(MessageType::RESPONSE_SET_CHUNK_INDEX);
                break;
            case MessageType::REQUEST_GET_CHUNK_HASH:
                seekToChunkIndex(m_chunkIndex);
                respondToClient(
                    MessageType::RESPONSE_GET_CHUNK_HASH,
                    Md5::calculateChunkHash(m_volumeDescriptor, m_chunkSize).toByteArray());
                break;
            case MessageType::REQUEST_WRITE_CHUNK_TO_TARGET:
                seekToChunkIndex(m_chunkIndex);
                if (payload.size() != m_chunkSize)
                {
                    throw std::runtime_error(
                        "unexpected size of chunk received "
                        "(" + std::to_string(payload.size()) + " != " + std::to_string(m_chunkSize) + ")");
                }
                Chunk::write(m_volumeDescriptor, payload);
                respondToClient(MessageType::RESPONSE_WRITE_CHUNK_TO_TARGET);
                break;
            default:
                throw std::runtime_error("unknown message type (" + std::to_string(messageHeader.messageType) + ")");
        }

        timeout.tv_sec = readTimeoutSecondsDefault;
        timeout.tv_usec = 0;
    }

    if (selectResult == 0)
    {
        throw std::runtime_error("select() timed out");
    }
    else if (selectResult < 0)
    {
        throw std::runtime_error("select() returned an error (" + std::string(strerror(errno)) + ")");
    }
}

void Server::respondToClient(MessageType response, const ByteArray& payload)
{
    auto messageHeader = MessageHeader(response, payload.size());
    // std::cerr << "SERVER TX >> " << messageHeader << std::endl;

    auto bytesWritten = write(STDOUT_FILENO, &messageHeader, sizeof(messageHeader));
    if (bytesWritten != (ssize_t) sizeof(messageHeader))
    {
        throw std::runtime_error(
            "unexpected number of message header bytes written "
            "(" + std::to_string(bytesWritten) + " != " + std::to_string(sizeof(messageHeader)) + ", " +
            std::string(strerror(errno)) + ")");
    }

    bytesWritten = write(STDOUT_FILENO, payload.c_str(), payload.size());
    if (bytesWritten != (ssize_t) payload.size())
    {
        throw std::runtime_error(
            "unexpected number of payload bytes written "
            "(" + std::to_string(bytesWritten) + " != " + std::to_string(payload.size()) + ", " +
            std::string(strerror(errno)) + ")");
    }
}

void Server::openVolume(void)
{
    closeVolume();

    auto descriptor = open(m_targetVolume.c_str(), O_RDWR);
    if (descriptor < 0)
    {
        throw std::runtime_error(
            "could not open volume \"" + m_targetVolume + "\" for writing (" + std::string(strerror(errno)) + ")");
    }

    m_volumeDescriptor = descriptor;
}

} /* namespace VolSync */
