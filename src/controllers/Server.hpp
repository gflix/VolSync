#ifndef CONTROLLERS_SERVER_HPP_
#define CONTROLLERS_SERVER_HPP_

#include <models/MessageType.hpp>
#include <utils/ByteArray.hpp>

namespace VolSync
{

class Server
{
public:
    Server(
        const std::string& targetVolume);
    virtual ~Server();

    void run(void);

    static constexpr time_t readTimeoutSecondsDefault = 10;
    static constexpr size_t receiveBufferMax = 48;

protected:
    const std::string& m_targetVolume;
    uint64_t m_chunkSize;
    uint64_t m_chunkIndex;

    void respondToClient(MessageType response, const ByteArray& payload = ByteArray());

private:

};

} /* namespace VolSync */

#endif /* CONTROLLERS_SERVER_HPP_ */
