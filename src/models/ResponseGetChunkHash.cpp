#include <cstring>
#include <stdexcept>
#include <models/ResponseGetChunkHash.hpp>

namespace VolSync
{

ResponseGetChunkHash::ResponseGetChunkHash(
    const ByteArray& md5Hash)
{
    bzero(this->md5Hash, sizeof(this->md5Hash));

    if (md5Hash.size() >= hashLength)
    {
        throw std::length_error("invalid hash length");
    }

    memcpy(this->md5Hash, md5Hash.c_str(), md5Hash.size());
}

ResponseGetChunkHash::~ResponseGetChunkHash()
{
}

ByteArray ResponseGetChunkHash::toByteArray(void) const
{
    return ByteArray((uint8_t*) this, sizeof(*this));
}

ResponseGetChunkHash ResponseGetChunkHash::fromByteArray(const ByteArray& data)
{
    if (data.size() != sizeof(ResponseGetChunkHash))
    {
        throw std::runtime_error(
            "unexpected number of data bytes "
            "(" + std::to_string(data.size()) + " != " + std::to_string(sizeof(ResponseGetChunkHash)) + ")");
    }

    ResponseGetChunkHash payload;
    memcpy(&payload, data.c_str(), data.size());

    return payload;
}

std::ostream& operator<<(std::ostream& stream, const ResponseGetChunkHash& item)
{
    stream << "ResponseGetChunkHash[";
    stream << "md5Hash=";
    for (auto i = 0; i < ResponseGetChunkHash::hashLength; ++i)
    {
        char buffer[4];
        snprintf(buffer, sizeof(buffer), "%02x", item.md5Hash[i]);
        stream << buffer;
    }
    stream << "]";

    return stream;
}

bool operator!=(const ResponseGetChunkHash& a, const ResponseGetChunkHash& b)
{
    auto aPointer = a.md5Hash;
    auto bPointer = b.md5Hash;
    for (auto i = 0; i < ResponseGetChunkHash::hashLength; ++i, ++aPointer, ++bPointer)
    {
        if (*aPointer != *bPointer)
        {
            return true;
        }
    }

    return false;
}

} /* namespace VolSync */
