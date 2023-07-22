#include <cstring>
#include <stdexcept>
#include <models/RequestSetChunkSize.hpp>

namespace VolSync
{

RequestSetChunkSize::RequestSetChunkSize(
    uint64_t size):
    size(size)
{
}

RequestSetChunkSize::~RequestSetChunkSize()
{
}

ByteArray RequestSetChunkSize::toByteArray(void) const
{
    return ByteArray((uint8_t*) this, sizeof(*this));
}

RequestSetChunkSize RequestSetChunkSize::fromByteArray(const ByteArray& data)
{
    if (data.size() != sizeof(RequestSetChunkSize))
    {
        throw std::runtime_error(
            "unexpected number of data bytes "
            "(" + std::to_string(data.size()) + " != " + std::to_string(sizeof(RequestSetChunkSize)) + ")");
    }

    RequestSetChunkSize payload;
    memcpy(&payload, data.c_str(), data.size());

    return payload;
}

std::ostream& operator<<(std::ostream& stream, const RequestSetChunkSize& item)
{
    stream << "RequestSetChunkSize[";
    stream << "size=" << item.size;
    stream << "]";

    return stream;
}

} /* namespace VolSync */
