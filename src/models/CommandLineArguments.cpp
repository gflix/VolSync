#include <models/CommandLineArguments.hpp>

namespace VolSync
{

CommandLineArguments::CommandLineArguments(
    const std::string& targetVolume,
    const std::string& remotePathToExecutable,
    const PositionalArguments& positionalArguments):
    targetVolume(targetVolume),
    remotePathToExecutable(remotePathToExecutable),
    positionalArguments(positionalArguments)
{
}

CommandLineArguments::~CommandLineArguments()
{
}

} /* namespace VolSync */
