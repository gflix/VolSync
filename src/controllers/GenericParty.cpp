#include <unistd.h>
#include <stdexcept>
#include <controllers/GenericParty.hpp>

namespace VolSync
{

GenericParty::GenericParty(void):
    m_volumeDescriptor(-1),
    m_chunkSize(0),
    m_chunkIndex(0)
{
}

GenericParty::~GenericParty()
{
    closeVolume();
}

void GenericParty::closeVolume(void)
{
    if (m_volumeDescriptor < 0)
    {
        return;
    }

    close(m_volumeDescriptor);
    m_volumeDescriptor = -1;
}

void GenericParty::checkVolumeIsOpen(void)
{
    if (m_volumeDescriptor < 0)
    {
        throw std::runtime_error("volume was not yet opened");
    }
}

void GenericParty::seekToChunkIndex(uint64_t chunkIndex)
{
    checkVolumeIsOpen();

    auto targetPosition = chunkIndex * m_chunkSize;
    auto actualPosition = lseek64(m_volumeDescriptor, targetPosition, SEEK_SET);

    if (actualPosition != targetPosition)
    {
        throw std::runtime_error("could not seek to position " + std::to_string(targetPosition));
    }
}

} /* namespace VolSync */
