#ifndef MODELS_REQUESTSETCHUNKSIZE_HPP_
#define MODELS_REQUESTSETCHUNKSIZE_HPP_

#include <cstdint>
#include <ostream>
#include <utils/ByteArray.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) RequestSetChunkSize
{
    explicit RequestSetChunkSize(
        uint64_t size = 0);
    ~RequestSetChunkSize();

    uint64_t size;

    ByteArray toByteArray(void) const;
    static RequestSetChunkSize fromByteArray(const ByteArray& data);
};

std::ostream& operator<<(std::ostream& stream, const RequestSetChunkSize& item);

} /* namespace VolSync */

#endif /* MODELS_REQUESTSETCHUNKSIZE_HPP_ */
