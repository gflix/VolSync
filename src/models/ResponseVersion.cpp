#include <cstring>
#include <stdexcept>
#include <models/ResponseVersion.hpp>

namespace VolSync
{

ResponseVersion::ResponseVersion(
    int protocolVersion,
    const std::string& packageName,
    const std::string& packageVersion):
    protocolVersion(protocolVersion)
{
    bzero(this->packageName, sizeof(this->packageName));
    bzero(this->packageVersion, sizeof(this->packageVersion));

    if (packageName.size() >= maxStringLength)
    {
        throw std::length_error("packageName exceeds the allowed maximum");
    }
    if (packageVersion.size() >= maxStringLength)
    {
        throw std::length_error("packageVersion exceeds the allowed maximum");
    }

    memcpy(this->packageName, packageName.c_str(), packageName.size());
    memcpy(this->packageVersion, packageVersion.c_str(), packageVersion.size());
}

ResponseVersion::~ResponseVersion()
{
}

ByteArray ResponseVersion::toByteArray(void) const
{
    return ByteArray((uint8_t*) this, sizeof(*this));
}

ResponseVersion ResponseVersion::fromByteArray(const ByteArray& data)
{
    if (data.size() != sizeof(ResponseVersion))
    {
        throw std::runtime_error(
            "unexpected number of data bytes "
            "(" + std::to_string(data.size()) + " != " + std::to_string(sizeof(ResponseVersion)) + ")");
    }

    ResponseVersion payload;
    memcpy(&payload, data.c_str(), data.size());

    if (payload.packageName[maxStringLength - 1] != 0 || payload.packageVersion[maxStringLength - 1] != 0)
    {
        throw std::runtime_error("strings are not zero-terminated");
    }

    return payload;
}

std::ostream& operator<<(std::ostream& stream, const ResponseVersion& item)
{
    stream << "ResponseVersion[";
    stream << "protocolVersion=" << item.protocolVersion << ",";
    stream << "packageName=" << item.packageName << ",";
    stream << "packageVersion=" << item.packageVersion;
    stream << "]";

    return stream;
}

} /* namespace VolSync */
