#ifndef MODELS_COMMANDLINEARGUMENTS_HPP_
#define MODELS_COMMANDLINEARGUMENTS_HPP_

#include <string>

namespace VolSync
{

struct CommandLineArguments
{
    explicit CommandLineArguments(
        bool isServer = false,
        const std::string& remotePathToExecutable = std::string());
    virtual ~CommandLineArguments();

    bool isServer;
    std::string remotePathToExecutable;
};

} /* namespace VolSync */

#endif /* MODELS_COMMANDLINEARGUMENTS_HPP_ */
