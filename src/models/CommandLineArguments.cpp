#include <models/CommandLineArguments.hpp>

namespace VolSync
{

CommandLineArguments::CommandLineArguments(
    bool showHelp,
    const std::string& sshCommand,
    const std::string& targetVolume,
    const std::string& remotePathToExecutable,
    bool verifyAfterWrite,
    int statusInterval,
    const PositionalArguments& positionalArguments):
    showHelp(showHelp),
    sshCommand(sshCommand),
    targetVolume(targetVolume),
    remotePathToExecutable(remotePathToExecutable),
    verifyAfterWrite(verifyAfterWrite),
    statusInterval(statusInterval),
    positionalArguments(positionalArguments)
{
}

CommandLineArguments::~CommandLineArguments()
{
}

} /* namespace VolSync */
