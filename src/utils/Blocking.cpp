#include <fcntl.h>
#include <fcntl.h>
#include <utils/Blocking.hpp>

namespace VolSync
{

void Blocking::setBlocking(int descriptor)
{
    int val;

    if ((val = fcntl(descriptor, F_GETFL)) == -1)
    {
        return;
    }

    if (val & O_NONBLOCK)
    {
        val &= ~O_NONBLOCK;
        fcntl(descriptor, F_SETFL, val);
    }
}

} /* namespace VolSync */
