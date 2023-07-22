#include <cstring>
#include <stdexcept>
#include <models/GenericPayloadWithSize.hpp>

namespace VolSync
{

GenericPayloadWithSize::GenericPayloadWithSize(
    uint64_t size):
    size(size)
{
}

GenericPayloadWithSize::~GenericPayloadWithSize()
{
}

ByteArray GenericPayloadWithSize::toByteArray(void) const
{
    return ByteArray((uint8_t*) this, sizeof(*this));
}

GenericPayloadWithSize GenericPayloadWithSize::fromByteArray(const ByteArray& data)
{
    if (data.size() != sizeof(GenericPayloadWithSize))
    {
        throw std::runtime_error(
            "unexpected number of data bytes "
            "(" + std::to_string(data.size()) + " != " + std::to_string(sizeof(GenericPayloadWithSize)) + ")");
    }

    GenericPayloadWithSize payload;
    memcpy(&payload, data.c_str(), data.size());

    return payload;
}

std::ostream& operator<<(std::ostream& stream, const GenericPayloadWithSize& item)
{
    stream << "GenericPayloadWithSize[";
    stream << "size=" << item.size;
    stream << "]";

    return stream;
}

} /* namespace VolSync */
