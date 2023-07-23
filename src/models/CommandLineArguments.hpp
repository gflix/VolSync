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
        bool showHelp = false,
        const std::string& sshCommand = sshCommandDefault,
        const std::string& targetVolume = std::string(),
        const std::string& remotePathToExecutable = std::string(),
        bool verifyAfterWrite = false,
        int statusInterval = statusIntervalSecondsDefault,
        const PositionalArguments& positionalArguments = PositionalArguments());
    virtual ~CommandLineArguments();

    bool showHelp;
    std::string sshCommand;
    std::string targetVolume;
    std::string remotePathToExecutable;
    bool verifyAfterWrite;
    int statusInterval;
    PositionalArguments positionalArguments;

    static constexpr const char* sshCommandDefault = "ssh";
    static constexpr int statusIntervalSecondsDefault = 15;
};

} /* namespace VolSync */

#endif /* MODELS_COMMANDLINEARGUMENTS_HPP_ */
