#ifndef CONTROLLERS_CLIENT_HPP_
#define CONTROLLERS_CLIENT_HPP_

#include <models/CommandLineArguments.hpp>

namespace VolSync
{

class Client
{
public:
    Client(
        const std::string& source,
        const std::string& target,
        const CommandLineArguments& commandLineArguments);
    virtual ~Client();

    void run(void);

protected:
    const std::string& m_sourceVolume;
    std::string m_remoteHost;
    std::string m_targetVolume;
    const CommandLineArguments& m_commandLineArguments;

private:

};

} /* namespace VolSync */

#endif /* CONTROLLERS_CLIENT_HPP_ */
