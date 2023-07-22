#include <models/CommandLineArguments.hpp>

namespace VolSync
{

CommandLineArguments::CommandLineArguments(
    bool showHelp,
    const std::string& sshCommand,
    const std::string& targetVolume,
    const std::string& remotePathToExecutable,
    const PositionalArguments& positionalArguments):
    showHelp(showHelp),
    sshCommand(sshCommand),
    targetVolume(targetVolume),
    remotePathToExecutable(remotePathToExecutable),
    positionalArguments(positionalArguments)
{
}

CommandLineArguments::~CommandLineArguments()
{
}

} /* namespace VolSync */
