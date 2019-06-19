#ifndef KIACONSOLE_OPTIONS_H
#define KIACONSOLE_OPTIONS_H

#include "pch.h"

#include <string>

class Options
{
public:
    bool valid;
    bool streaming;
    std::wstring directory;

    Options(int argc, char **argv);
    void usage();
};

#endif
