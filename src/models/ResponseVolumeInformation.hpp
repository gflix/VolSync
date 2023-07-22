#ifndef MODELS_RESPONSEVOLUMEINFORMATION_HPP_
#define MODELS_RESPONSEVOLUMEINFORMATION_HPP_

#include <cstdint>
#include <ostream>
#include <utils/ByteArray.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) ResponseVolumeInformation
{
    explicit ResponseVolumeInformation(
        uint64_t size = 0);
    ~ResponseVolumeInformation();

    uint64_t size;

    ByteArray toByteArray(void) const;
    static ResponseVolumeInformation fromByteArray(const ByteArray& data);
};

std::ostream& operator<<(std::ostream& stream, const ResponseVolumeInformation& item);

} /* namespace VolSync */

#endif /* MODELS_RESPONSEVOLUMEINFORMATION_HPP_ */
