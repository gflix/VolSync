#include <unistd.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <controllers/Server.hpp>
#include <models/MessageHeader.hpp>
#include <models/ResponseVersion.hpp>
#include <utils/ByteArray.hpp>

namespace VolSync
{

Server::Server()
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

        if (messageHeader.payloadLength > receiveBufferMax)
        {
            throw std::runtime_error(
                "actual payload length exceeds the allowed receive buffer "
                "(" + std::to_string(messageHeader.payloadLength) + " > " + std::to_string(receiveBufferMax) + ")");
        }

        ByteArray payload;
        if (messageHeader.payloadLength > 0)
        {
            auto buffer = std::make_unique<uint8_t>((int) messageHeader.payloadLength);
            bytesRead = read(STDIN_FILENO, buffer.get(), messageHeader.payloadLength);

            if (bytesRead <= 0 || bytesRead != (ssize_t) messageHeader.payloadLength)
            {
                throw std::runtime_error(
                    "unexpected number of payload bytes read "
                    "(" + std::to_string(bytesRead) + " != " + std::to_string(messageHeader.payloadLength) + ", " +
                    std::string(strerror(errno)) + ")");
            }

            payload = ByteArray(buffer.get(), bytesRead);
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
            default:
                throw std::runtime_error("unknown message type");
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

} /* namespace VolSync */
