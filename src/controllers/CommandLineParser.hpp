#ifndef CONTROLLERS_COMMANDLINEPARSER_HPP_
#define CONTROLLERS_COMMANDLINEPARSER_HPP_

#include <ostream>
#include <models/CommandLineArguments.hpp>

namespace VolSync
{

class CommandLineParser
{
public:
    static CommandLineArguments parse(int argc, char const* argv[]);
    static void usage(std::ostream& stream, const std::string& programName);
};

} /* namespace VolSync */

#endif /* CONTROLLERS_COMMANDLINEPARSER_HPP_ */
