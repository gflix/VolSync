#ifndef CONTROLLERS_CLIENT_HPP_
#define CONTROLLERS_CLIENT_HPP_

#include <ostream>
#include <controllers/GenericParty.hpp>
#include <models/CommandLineArguments.hpp>
#include <models/MessageType.hpp>
#include <models/ResponseGetChunkHash.hpp>
#include <models/ResponseVersion.hpp>
#include <models/ResponseVolumeInformation.hpp>
#include <utils/ByteArray.hpp>

namespace VolSync
{

class Client: public GenericParty
{
public:
    typedef std::vector<std::string> ProcessArguments;

    struct Child
    {
        Child(
            int readDescriptor,
            int writeDescriptor);

        int readDescriptor;
        int writeDescriptor;
    };

    Client(
        const std::string& source,
        const std::string& target,
        const CommandLineArguments& commandLineArguments);
    virtual ~Client();

    void run(void) override;

    static constexpr const char* remoteCommandDefault = "vol-sync";
    static constexpr uint64_t chunkSizeMin = 512;
    static constexpr uint64_t chunkSizeMax = 1024*1024;

protected:
    const std::string& m_sourceVolume;
    std::string m_remoteHost;
    std::string m_targetVolume;
    const CommandLineArguments& m_commandLineArguments;

    void openVolume(void) override;

    static ResponseVersion getServerVersion(const Child& child);
    static ResponseVolumeInformation getTargetVolumeInformation(const Child& child);
    static void requestOpenVolume(const Child& child);
    static void requestCloseVolume(const Child& child);
    static void setChunkSize(const Child& child, uint64_t chunkSize);
    static void setChunkIndex(const Child& child, uint64_t chunkIndex);
    static ResponseGetChunkHash getChunkHash(const Child& child);
    static void writeChunkToTarget(const Child& child, const ByteArray& chunk);
    static void requestAbort(const Child& child);

    static void doSimpleRequest(
        const Child& child,
        MessageType request,
        MessageType expectedResponse);
    static void communicateWithServer(
        const Child& child,
        MessageType request,
        const ByteArray& requestPayload,
        MessageType expectedResponse,
        ByteArray& responsePayload);

    static Child startRemoteServer(
        const std::string& sshCommand,
        const std::string& remoteHost,
        const std::string& remotePathToExecutable,
        const std::string& targetVolume);
    static Child startRemoteServer(
        const ProcessArguments& arguments);

    static ProcessArguments splitCommand(const std::string& command);
    static uint64_t determineBestChunkSize(uint64_t volumeSize);

private:

};

std::ostream& operator<<(std::ostream& stream, const Client::ProcessArguments& arguments);

} /* namespace VolSync */

#endif /* CONTROLLERS_CLIENT_HPP_ */
