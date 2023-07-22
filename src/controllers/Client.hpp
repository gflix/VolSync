#ifndef CONTROLLERS_CLIENT_HPP_
#define CONTROLLERS_CLIENT_HPP_

#include <ostream>
#include <models/CommandLineArguments.hpp>
#include <models/MessageType.hpp>
#include <utils/ByteArray.hpp>

namespace VolSync
{

class Client
{
public:
    typedef std::vector<std::string> ProcessArguments;

    Client(
        const std::string& source,
        const std::string& target,
        const CommandLineArguments& commandLineArguments);
    virtual ~Client();

    void run(void);

    static constexpr const char* remoteCommandDefault = "vol-sync";
    static constexpr time_t readTimeoutSecondsDefault = 10;
    static constexpr time_t receiveBufferMax = 16;

protected:
    const std::string& m_sourceVolume;
    std::string m_remoteHost;
    std::string m_targetVolume;
    const CommandLineArguments& m_commandLineArguments;

    void communicateWithServer(
        int descriptorToChild,
        int descriptorFromChild,
        MessageType request,
        const ByteArray& requestPayload,
        MessageType expectedResponse,
        ByteArray& responsePayload);

    static void startRemoteServer(
        const std::string& sshCommand,
        const std::string& remoteHost,
        const std::string& remotePathToExecutable,
        const std::string& targetVolume,
        int& descriptorToChild,
        int& descriptorFromChild);
    static void startRemoteServer(
        const ProcessArguments& arguments,
        int& descriptorToChild,
        int& descriptorFromChild);

    static ProcessArguments splitCommand(const std::string& command);

private:

};

std::ostream& operator<<(std::ostream& stream, const Client::ProcessArguments& arguments);

} /* namespace VolSync */

#endif /* CONTROLLERS_CLIENT_HPP_ */
