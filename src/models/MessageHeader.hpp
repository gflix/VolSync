#ifndef MODELS_MESSAGEHEADER_HPP_
#define MODELS_MESSAGEHEADER_HPP_

#include <cstdint>
#include <ostream>
#include <models/MessageType.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) MessageHeader
{
    explicit MessageHeader(
        MessageType messageType = MessageType::INVALID,
        uint32_t payloadLength = 0);
    ~MessageHeader();  // not defined as virtual to omit the vtable

    uint32_t messageType;
    uint32_t payloadLength;
};

std::ostream& operator<<(std::ostream& stream, const MessageHeader& header);

} /* namespace VolSync */

#endif /* MODELS_MESSAGEHEADER_HPP_ */
