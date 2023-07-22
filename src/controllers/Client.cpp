#include <stdexcept>
#include <controllers/Client.hpp>

namespace VolSync
{

Client::Client(
    const std::string& source,
    const std::string& target,
    const CommandLineArguments& commandLineArguments):
    m_sourceVolume(source),
    m_commandLineArguments(commandLineArguments)
{
    auto colonPosition = target.find(':');
    if (colonPosition == target.npos)
    {
        throw std::invalid_argument("invalid target format");
    }

    m_remoteHost = target.substr(0, colonPosition);
    m_targetVolume = target.substr(colonPosition + 1);

    if (m_remoteHost.empty())
    {
        throw std::invalid_argument("no remote host given");
    }
    if (m_targetVolume.empty())
    {
        throw std::invalid_argument("no target volume given");
    }
    if (m_targetVolume[0] != '/')
    {
        throw std::invalid_argument("target volume must be an absolute path");
    }
}

Client::~Client()
{
}

void Client::run(void)
{
    throw std::runtime_error("not yet implemented [" + m_remoteHost + "][" + m_targetVolume + "]");
}

} /* namespace VolSync */
