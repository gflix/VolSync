#include <cstring>
#include <stdexcept>
#include <models/RequestSetChunkIndex.hpp>

namespace VolSync
{

RequestSetChunkIndex::RequestSetChunkIndex(
    uint64_t chunkIndex):
    chunkIndex(chunkIndex)
{
}

RequestSetChunkIndex::~RequestSetChunkIndex()
{
}

ByteArray RequestSetChunkIndex::toByteArray(void) const
{
    return ByteArray((uint8_t*) this, sizeof(*this));
}

RequestSetChunkIndex RequestSetChunkIndex::fromByteArray(const ByteArray& data)
{
    if (data.size() != sizeof(RequestSetChunkIndex))
    {
        throw std::runtime_error(
            "unexpected number of data bytes "
            "(" + std::to_string(data.size()) + " != " + std::to_string(sizeof(RequestSetChunkIndex)) + ")");
    }

    RequestSetChunkIndex payload;
    memcpy(&payload, data.c_str(), data.size());

    return payload;
}

std::ostream& operator<<(std::ostream& stream, const RequestSetChunkIndex& item)
{
    stream << "RequestSetChunkIndex[";
    stream << "chunkIndex=" << item.chunkIndex;
    stream << "]";

    return stream;
}

} /* namespace VolSync */
