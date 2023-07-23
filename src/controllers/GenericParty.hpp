#ifndef CONTROLLERS_GENERICPARTY_HPP_
#define CONTROLLERS_GENERICPARTY_HPP_

namespace VolSync
{

class GenericParty
{
public:
    GenericParty(void);
    virtual ~GenericParty();

    virtual void run(void) = 0;

    static constexpr time_t readTimeoutSecondsDefault = 10;
    static constexpr size_t receiveBufferMax = 256*1024;

protected:
    int m_volumeDescriptor;
    uint64_t m_chunkSize;
    uint64_t m_chunkIndex;

    virtual void openVolume(void) = 0;
    void closeVolume(void);
    void checkVolumeIsOpen(void);
    void seekToChunkIndex(uint64_t chunkIndex);

private:

};

} /* namespace VolSync */

#endif /* CONTROLLERS_GENERICPARTY_HPP_ */
