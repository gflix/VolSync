#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <utils/Chunk.hpp>

namespace VolSync
{

ByteArray Chunk::read(int descriptor, uint64_t chunkSize)
{
    uint64_t buffer[chunkSize];
    auto bytesRead = ::read(descriptor, buffer, chunkSize);

    if (bytesRead != (ssize_t) chunkSize)
    {
        throw std::runtime_error(
            "unexpected number of bytes read from volume "
            "(" + std::to_string(bytesRead) + " != " + std::to_string(chunkSize) + ", " +
            std::string(strerror(errno)) + ")");
    }

    return ByteArray((uint8_t*) buffer, (size_t) chunkSize);
}

void Chunk::write(int descriptor, const ByteArray& data)
{
    auto bytesWritten = ::write(descriptor, data.c_str(), data.size());

    if (bytesWritten != (ssize_t) data.size())
    {
        throw std::runtime_error(
            "unexpected number of bytes written to volume "
            "(" + std::to_string(bytesWritten) + " != " + std::to_string(data.size()) + ", " +
            std::string(strerror(errno)) + ")");
    }
}

} /* namespace VolSync */
