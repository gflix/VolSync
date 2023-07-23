#ifndef UTILS_MD5_HPP_
#define UTILS_MD5_HPP_

#include <models/ResponseGetChunkHash.hpp>

namespace VolSync
{

class Md5
{
public:
    static ResponseGetChunkHash calculateChunkHash(int descriptor, uint64_t chunkSize);
};

} /* namespace VolSync */

#endif /* UTILS_MD5_HPP_ */
