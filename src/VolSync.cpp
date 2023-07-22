#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <controllers/CommandLineParser.hpp>

using namespace std;
using namespace VolSync;

void runInClientMode(void)
{
    throw runtime_error("not yet implemented");
}

void runInSeverMode(void)
{
    throw runtime_error("not yet implemented");
}

int main(int argc, char const* argv[])
{
    CommandLineArguments commandLineArguments;
    std::string source;
    std::string target;
    try
    {
        commandLineArguments = CommandLineParser::parse(argc, argv);
        if (commandLineArguments.targetVolume.empty())
        {
            if (commandLineArguments.positionalArguments.size() < 2)
            {
                throw invalid_argument("missing mandatory arguments");
            }
            source = commandLineArguments.positionalArguments[0];
            target = commandLineArguments.positionalArguments[1];
        }
    }
    catch(const std::exception& e)
    {
        cerr << "ERROR: Could not parse the command line arguments (" << e.what() << ")! Aborting." << endl;
        CommandLineParser::usage(cerr, argv[0]);
        return 1;
    }

    if (commandLineArguments.targetVolume.empty())
    {
        try
        {
            runInClientMode();
        }
        catch(const std::exception& e)
        {
            cerr << "ERROR: Encountered an error on the local host (" << e.what() << ")! Aborting." << endl;
            return 1;
        }
    }
    else
    {
        try
        {
            runInSeverMode();
        }
        catch(const std::exception& e)
        {
            cerr << "ERROR: Encountered an error on the remote host (" << e.what() << ")! Aborting." << endl;
            return 1;
        }
    }

    return 0;
}
