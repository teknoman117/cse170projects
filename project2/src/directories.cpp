#include <project2/directories.hpp>

#ifndef __APPLE__

std::string GetApplicationResourcesDirectory()
{
    return std::string(".");
}

#endif
