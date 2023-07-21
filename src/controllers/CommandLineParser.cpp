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
    while ((getoptReturnValue = getopt(argc, (char * const *) argv, "sr:")) != -1)
    {
        switch (getoptReturnValue)
        {
            case 's':
                arguments.isServer = true;
                break;
            case 'r':
                arguments.remotePathToExecutable = optarg;
                if (!arguments.remotePathToExecutable.empty())
                {
                    auto pathLength = arguments.remotePathToExecutable.size();
                    if (arguments.remotePathToExecutable[0] != '/')
                    {
                        throw std::domain_error("remote path must be an absolute path");
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

    return arguments;
}

void CommandLineParser::usage(std::ostream& stream, const std::string& programName)
{
    stream << "USAGE:  " << programName << " [OPTIONS]" << std::endl;
    stream << std::endl;
    stream << "Optional arguments:" << std::endl;
    stream << "  -r PATH  Absolute path to the vol-sync executable on the remote machine" << std::endl;
    stream << "           (default: none to use the search path)" << std::endl;
    stream << "  -s       Run in server mode (internally used when connecting to a remote machine)" << std::endl;
}

} /* namespace VolSync */
