#include <models/MessageHeader.hpp>

namespace VolSync
{

MessageHeader::MessageHeader(
    MessageType messageType,
    uint32_t payloadLength):
    messageType(static_cast<uint32_t>(messageType)),
    payloadLength(payloadLength)
{
}

MessageHeader::~MessageHeader()
{
}

std::ostream& operator<<(std::ostream& stream, const MessageHeader& header)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%02x", header.messageType);
    stream << "MessageHeader[";
    stream << "messageType=" << buffer << ",";
    snprintf(buffer, sizeof(buffer), "%d", header.payloadLength);
    stream << "payloadLength=" << buffer;
    stream << "]";

    return stream;
}

} /* namespace VolSync */
