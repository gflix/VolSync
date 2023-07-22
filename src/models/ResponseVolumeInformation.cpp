#include <cstring>
#include <stdexcept>
#include <models/ResponseVolumeInformation.hpp>

namespace VolSync
{

ResponseVolumeInformation::ResponseVolumeInformation(
    uint64_t size):
    size(size)
{
}

ResponseVolumeInformation::~ResponseVolumeInformation()
{
}

ByteArray ResponseVolumeInformation::toByteArray(void) const
{
    return ByteArray((uint8_t*) this, sizeof(*this));
}

ResponseVolumeInformation ResponseVolumeInformation::fromByteArray(const ByteArray& data)
{
    if (data.size() != sizeof(ResponseVolumeInformation))
    {
        throw std::runtime_error(
            "unexpected number of data bytes "
            "(" + std::to_string(data.size()) + " != " + std::to_string(sizeof(ResponseVolumeInformation)) + ")");
    }

    ResponseVolumeInformation payload;
    memcpy(&payload, data.c_str(), data.size());

    return payload;
}

std::ostream& operator<<(std::ostream& stream, const ResponseVolumeInformation& item)
{
    stream << "ResponseVolumeInformation[";
    stream << "size=" << item.size;
    stream << "]";

    return stream;
}

} /* namespace VolSync */
