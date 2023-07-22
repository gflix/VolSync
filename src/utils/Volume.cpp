#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <cstring>
#include <stdexcept>
#include <utils/Volume.hpp>

namespace VolSync
{

ResponseVolumeInformation Volume::getInformation(const std::string& volume)
{
    struct stat volumeStat;
    auto statResult = stat(volume.c_str(), &volumeStat);

    if (statResult < 0)
    {
        throw std::runtime_error("could not stat() volume \"" + volume + "\"");
    }

    if ((volumeStat.st_mode & S_IFMT) == S_IFREG)
    {
        return ResponseVolumeInformation(volumeStat.st_size);
    }
    else if ((volumeStat.st_mode & S_IFMT) == S_IFBLK)
    {
        auto descriptor = open(volume.c_str(), O_RDONLY);
        if (descriptor < 0)
        {
            throw std::runtime_error(
                "could not open volume \"" + volume + "\" for reading (" + std::string(strerror(errno)) + ")");
        }

        ResponseVolumeInformation volumeInformation;
        if (ioctl(descriptor, BLKGETSIZE64, &volumeInformation.size) != 0)
        {
            close(descriptor);
            throw std::runtime_error(
                "could not request volume \"" + volume + "\" size (" + std::string(strerror(errno)) + ")");
        }
        close(descriptor);

        return volumeInformation;
    }

    throw std::runtime_error("volume \"" + volume + "\" is not supported");
}

} /* namespace VolSync */
