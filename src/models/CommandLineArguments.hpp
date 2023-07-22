#ifndef MODELS_COMMANDLINEARGUMENTS_HPP_
#define MODELS_COMMANDLINEARGUMENTS_HPP_

#include <string>
#include <vector>

namespace VolSync
{

struct CommandLineArguments
{
    typedef std::vector<std::string> PositionalArguments;

    explicit CommandLineArguments(
        const std::string& targetVolume = std::string(),
        const std::string& remotePathToExecutable = std::string(),
        const PositionalArguments& positionalArguments = PositionalArguments());
    virtual ~CommandLineArguments();

    std::string targetVolume;
    std::string remotePathToExecutable;
    PositionalArguments positionalArguments;
};

} /* namespace VolSync */

#endif /* MODELS_COMMANDLINEARGUMENTS_HPP_ */
