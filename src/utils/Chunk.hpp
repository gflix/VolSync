#ifndef UTILS_CHUNK_HPP_
#define UTILS_CHUNK_HPP_

#include <utils/ByteArray.hpp>

namespace VolSync
{

class Chunk
{
public:
    static ByteArray read(int descriptor, uint64_t chunkSize);
    static void write(int descriptor, const ByteArray& data);
};

} /* namespace VolSync */

#endif /* UTILS_CHUNK_HPP_ */
