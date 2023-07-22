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

} /* namespace VolSync */
