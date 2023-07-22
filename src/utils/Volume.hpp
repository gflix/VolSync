#ifndef UTILS_VOLUME_HPP_
#define UTILS_VOLUME_HPP_

#include <string>
#include <models/ResponseVolumeInformation.hpp>

namespace VolSync
{

class Volume
{
public:
    static ResponseVolumeInformation getInformation(const std::string& volume);
};

} /* namespace VolSync */

#endif /* UTILS_VOLUME_HPP_ */
