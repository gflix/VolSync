#ifndef MODELS_RESPONSEGETCHUNKHASH_HPP_
#define MODELS_RESPONSEGETCHUNKHASH_HPP_

#include <cstdint>
#include <ostream>
#include <string>
#include <utils/ByteArray.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) ResponseGetChunkHash
{
    explicit ResponseGetChunkHash(
        const ByteArray& md5Hash = ByteArray());
    ~ResponseGetChunkHash();

    static constexpr int hashLength = 16;

    uint8_t md5Hash[hashLength];

    ByteArray toByteArray(void) const;
    static ResponseGetChunkHash fromByteArray(const ByteArray& data);
};

std::ostream& operator<<(std::ostream& stream, const ResponseGetChunkHash& item);
bool operator!=(const ResponseGetChunkHash& a, const ResponseGetChunkHash& b);

} /* namespace VolSync */

#endif /* MODELS_RESPONSEGETCHUNKHASH_HPP_ */
