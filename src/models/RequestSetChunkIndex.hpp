#ifndef MODELS_REQUESTSETCHUNKINDEX_HPP_
#define MODELS_REQUESTSETCHUNKINDEX_HPP_

#include <cstdint>
#include <ostream>
#include <utils/ByteArray.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) RequestSetChunkIndex
{
    explicit RequestSetChunkIndex(
        uint64_t chunkIndex = 0);
    ~RequestSetChunkIndex();

    uint64_t chunkIndex;

    ByteArray toByteArray(void) const;
    static RequestSetChunkIndex fromByteArray(const ByteArray& data);
};

std::ostream& operator<<(std::ostream& stream, const RequestSetChunkIndex& item);

} /* namespace VolSync */

#endif /* MODELS_REQUESTSETCHUNKINDEX_HPP_ */
