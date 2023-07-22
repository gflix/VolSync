#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <controllers/Client.hpp>
#include <controllers/CommandLineParser.hpp>
#include <controllers/Server.hpp>

using namespace std;
using namespace VolSync;



int main(int argc, char const* argv[])
{
    CommandLineArguments commandLineArguments;
    std::string source;
    std::string target;
    try
    {
        commandLineArguments = CommandLineParser::parse(argc, argv);

        if (commandLineArguments.showHelp)
        {
            CommandLineParser::usage(cerr, argv[0]);
            return 0;
        }

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
            Client client { source, target, commandLineArguments };
            client.run();
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
            Server server;
            server.run();
        }
        catch(const std::exception& e)
        {
            cerr << "ERROR: Encountered an error on the remote host (" << e.what() << ")! Aborting." << endl;
            return 1;
        }
    }

    return 0;
}
