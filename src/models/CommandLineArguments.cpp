#include <models/CommandLineArguments.hpp>

namespace VolSync
{

CommandLineArguments::CommandLineArguments(
    bool isServer,
    const std::string& remotePathToExecutable):
    isServer(isServer),
    remotePathToExecutable(remotePathToExecutable)
{
}

CommandLineArguments::~CommandLineArguments()
{
}

} /* namespace VolSync */
