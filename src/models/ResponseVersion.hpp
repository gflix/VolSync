#ifndef MODELS_RESPONSEVERSION_HPP_
#define MODELS_RESPONSEVERSION_HPP_

#include <cstdint>
#include <ostream>
#include <string>
#include <utils/ByteArray.hpp>

namespace VolSync
{

struct __attribute__ ((packed)) ResponseVersion
{
    explicit ResponseVersion(
        int protocolVersion = protocolVersionImplemented,
        const std::string& packageName = PACKAGE_NAME,
        const std::string& packageVersion = PACKAGE_VERSION);
    ~ResponseVersion();

    static constexpr int protocolVersionImplemented = 1;
    static constexpr int maxStringLength = 16;

    uint32_t protocolVersion;
    char packageName[maxStringLength];
    char packageVersion[maxStringLength];

    ByteArray toByteArray(void) const;
    static ResponseVersion fromByteArray(const ByteArray& data);
};

std::ostream& operator<<(std::ostream& stream, const ResponseVersion& item);

} /* namespace VolSync */

#endif /* MODELS_RESPONSEVERSION_HPP_ */
