#include "pch.h"

#include "Measurement.h"

#include "Util.h"

#include <exception>
#include <iostream>
#include <istream>

using std::string;
using std::istream;

Measurement::Measurement(const std::wstring& pathname)
    : pathname(pathname)
{
    std::ifstream infile(pathname);
    load(infile);
}

Measurement::Measurement(int pixels)
{
    load(std::cin, pixels);
    valid = x.size() == pixels;
}

void Measurement::load(istream& is, int pixels)
{
    valid = false;

    char buf[256] = { 0 };
    int linecount = 0;
    while (is.getline(buf, sizeof(buf)))
    {
        string line(buf);
        Util::trim(line);

        // WHY do I have to convert the line to wstring before calling vwprintf?  Docs say %s should work with narrow string...
        // Util::log(L"read line [%04d / %d]: [%s]", linecount, pixels, Util::toWstring(line.c_str()).c_str());
        
        linecount++;

        // skip blanks and comments
        if (line.size() == 0 || line[0] == '#' || line[0] == '/' || line[0] == '\'')
            continue;

        // skip lines that don't start with a digit
        if (!(('0' <= line[0] && line[0] <= '9') || line[0] == '-'))
            continue;

        // expect lines to be "x, y" pairs
        try
        {
            std::vector<string> tokens = Util::split(line, ",");
            if (tokens.size() > 1)
            {
                x.push_back(stod(tokens[0]));
                y.push_back(stod(tokens[1]));

                if (pixels > 0 && x.size() == pixels)
                {
                    Util::log(L"Read expected %d pixels", pixels);
                    break;
                }
            }
            else
            {
                printf("ERROR parsing line from %ls: %s\n", pathname.c_str(), line.c_str());
                return;
            }
        }
        catch (std::exception &e)
        {
            printf("ERROR parsing line from %ls: %s: %s\n", pathname.c_str(), line.c_str(), e.what());
            return;
        }
    }
    Util::log(L"Finished loading measurement");

    valid = true;
}

bool Measurement::isValid() const
{
    return valid && x.size() == y.size() && x.size() > 1;
}
