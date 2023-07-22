#ifndef MODELS_MESSAGETYPE_HPP_
#define MODELS_MESSAGETYPE_HPP_

namespace VolSync
{

enum class MessageType
{
    INVALID = 0x00,
    REQUEST_ABORT = 0x10,
    RESPONSE_ABORT = 0x11,
    REQUEST_VERSION = 0x12,
    RESPONSE_VERSION = 0x13,
};

} /* namespace VolSync */

#endif /* MODELS_MESSAGETYPE_HPP_ */
