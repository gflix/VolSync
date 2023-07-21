#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <controllers/CommandLineParser.hpp>

using namespace std;
using namespace VolSync;

typedef std::vector<std::string> ProcessArguments;

void startProcessWithRedirection(const ProcessArguments& arguments, int& toChild, int& fromChild)
{
    // const char * const arguments[] = {
    //     "ssh",
    //     "-p",
    //     "22",
    //     "localhost",
    //     "tr",
    //     "a-z",
    //     "A-Z",
    //     nullptr };

    int toChildPipe[2];
	int fromChildPipe[2];

    if (pipe(toChildPipe) < 0 || pipe(fromChildPipe) < 0)
    {
        throw runtime_error("could not set up pipes");
    }

    auto pid = fork();

    if (pid < 0)
    {
        throw runtime_error("could not fork (" + string(strerror(errno)) + ")");
    }

    if (pid == 0)
    {
        if (dup2(toChildPipe[0], STDIN_FILENO) < 0 ||
            close(toChildPipe[1]) < 0 ||
            close(fromChildPipe[0]) < 0 ||
            dup2(fromChildPipe[1], STDOUT_FILENO) < 0)
        {
            throw std::runtime_error("could not duplicate/close the pipes (" + string(strerror(errno)) + ")");
        }

        if (toChildPipe[0] != STDIN_FILENO)
        {
            close(toChildPipe[0]);
        }
        if (fromChildPipe[1] != STDOUT_FILENO)
        {
            close(fromChildPipe[1]);
        }

        const char* argumentList[arguments.size() + 1];
        unsigned int i = 0;
        for (; i < arguments.size(); ++i)
        {
            argumentList[i] = arguments[i].c_str();
        }
        argumentList[i] = nullptr;


        if (execvp(argumentList[0], (char * const *) argumentList))
        {
            throw runtime_error("could not run command (" + string(strerror(errno)) + ")");
        }
    }

    close(toChildPipe[0]);
    close(fromChildPipe[1]);

    toChild = toChildPipe[1];
    fromChild = fromChildPipe[0];
}

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

    int toChild;
    int fromChild;

    startProcessWithRedirection({ "ssh", "-p", "22", "localhost", "tr", "a-z", "A-Z" }, toChild, fromChild);

    char buf[16];
    snprintf(buf, sizeof(buf), "aBcdE");

    auto bytesWritten = write(toChild, buf, strlen(buf));

    cout << "bytesWritten=" << bytesWritten << endl;

    if (bytesWritten < 0)
    {
        throw runtime_error("could not send bytes to the child");
    }
    close(toChild);

    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(fromChild, &descriptors);
    timeval timeout { 30, 0 };

    auto selectResult = select(fromChild + 1, &descriptors, nullptr, nullptr, &timeout);
    cout << "selectResult= " << selectResult << endl;
    if (selectResult > 0 && FD_ISSET(fromChild, &descriptors))
    {
        auto bytesRead = read(fromChild, buf, bytesWritten);

        if (bytesRead <= 0)
        {
            throw runtime_error("child died unexpectedly");
        }

        cout << "bytesRead=   " << bytesRead << endl;
        cout << "[" << buf << "]" << endl;
    }

    close(fromChild);

    return 0;
}
