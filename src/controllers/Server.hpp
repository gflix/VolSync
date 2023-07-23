#ifndef CONTROLLERS_SERVER_HPP_
#define CONTROLLERS_SERVER_HPP_

#include <controllers/GenericParty.hpp>
#include <models/MessageType.hpp>
#include <utils/ByteArray.hpp>

namespace VolSync
{

class Server: public GenericParty
{
public:
    Server(
        const std::string& targetVolume);
    virtual ~Server();

    void run(void) override;

protected:
    const std::string& m_targetVolume;

    void respondToClient(MessageType response, const ByteArray& payload = ByteArray());
    void openVolume(void) override;

private:

};

} /* namespace VolSync */

#endif /* CONTROLLERS_SERVER_HPP_ */
