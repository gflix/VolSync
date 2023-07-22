#include <getopt.h>
#include <stdexcept>
#include <controllers/CommandLineParser.hpp>

namespace VolSync
{

CommandLineArguments CommandLineParser::parse(int argc, char const* argv[])
{
    CommandLineArguments arguments;

    optind = 1;
    int getoptReturnValue;
    while ((getoptReturnValue = getopt(argc, (char * const *) argv, "e:s:r:")) != -1)
    {
        switch (getoptReturnValue)
        {
            case 'e':
                arguments.sshCommand = optarg;
                break;
            case 's':
                arguments.targetVolume = optarg;
                if (arguments.targetVolume.empty())
                {
                    throw std::invalid_argument("empty target volume is not allowed");
                }
                else
                {
                    if (arguments.targetVolume[0] != '/')
                    {
                        throw std::domain_error("target volume needs to be an absolute path");
                    }
                }
                break;
            case 'r':
                arguments.remotePathToExecutable = optarg;
                if (!arguments.remotePathToExecutable.empty())
                {
                    auto pathLength = arguments.remotePathToExecutable.size();
                    if (arguments.remotePathToExecutable[0] != '/')
                    {
                        throw std::domain_error("remote path needs to be an absolute path");
                    }

                    if (pathLength > 1 && arguments.remotePathToExecutable[pathLength - 1] != '/')
                    {
                        arguments.remotePathToExecutable += '/';
                    }
                }
                break;
            default:
                throw std::invalid_argument("invalid argument");
        }
    }

    while (optind < argc)
    {
        arguments.positionalArguments.push_back(argv[optind]);
        ++optind;
    }

    return arguments;
}

void CommandLineParser::usage(std::ostream& stream, const std::string& programName)
{
    stream << "USAGE:  " << programName << " [OPTIONS] [SOURCE TARGET]" << std::endl;
    stream << std::endl;
    stream << "Mandatory arguments in client mode:" << std::endl;
    stream << "  SOURCE      Source volume (device) on the local host" << std::endl;
    stream << "  TARGET      Target volume (device) on the remote host," << std::endl;
    stream << "              given as [USER@]HOST:VOLUME" << std::endl;
    stream << std::endl;
    stream << "Optional arguments:" << std::endl;
    stream << "  -e COMMAND  SSH command (default: " << CommandLineArguments::sshCommandDefault << ")" << std::endl;
    stream << "  -r PATH     Absolute path to the vol-sync executable on the remote machine" << std::endl;
    stream << "              (default: none to use the search path)" << std::endl;
    stream << "  -s TARGET   Run in server mode and use the given target volume (device)" << std::endl;
    stream << "              (internally used when connecting to a remote machine)" << std::endl;
    stream << std::endl;
    stream << "Version information:" << std::endl;
    stream << "  " << PACKAGE_NAME << " " << PACKAGE_VERSION << std::endl;
}

} /* namespace VolSync */
