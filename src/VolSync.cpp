#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <controllers/CommandLineParser.hpp>

using namespace std;
using namespace VolSync;

int main(int argc, char const* argv[])
{
    CommandLineArguments commandLineArguments;
    try
    {
        commandLineArguments = CommandLineParser::parse(argc, argv);
    }
    catch(const std::exception& e)
    {
        cerr << "ERROR: Could not parse the command line arguments (" << e.what() << ")! Aborting." << endl;
        CommandLineParser::usage(cerr, argv[0]);
        return 1;
    }

    return 0;
}
