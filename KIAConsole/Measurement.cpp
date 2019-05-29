#include "pch.h"

#include "Measurement.h"

#include "Util.h"

#include <fstream>
#include <exception>

using std::string;

Measurement::Measurement(const std::wstring& pathname)
    : pathname(pathname)
{
    load();
}

void Measurement::load()
{
    std::ifstream infile(pathname);
    valid = false;

    string line;
    while (std::getline(infile, line))
    {
        Util::trim(line);

        // skip blanks and comments
        if (line.size() == 0 || line[0] == '#' || line[0] == '/' || line[0] == '\'')
            continue;

        // expect lines to be "x, y" pairs
        try
        {
            std::vector<string> tokens = Util::split(line, ",");
            if (tokens.size() > 1)
            {
                x.push_back(stod(tokens[0]));
                y.push_back(stod(tokens[1]));
            }
            else
            {
                printf("ERROR parsing line from %ls: %s\n", pathname.c_str(), line.c_str());
                return;
            }
        }
        catch (std::exception &e)
        {
            printf("ERROR parsing line from %ls: %s\n", pathname.c_str(), line.c_str());
            return;
        }
    }

    valid = true;
}

bool Measurement::isValid() const
{
    return valid && x.size() == y.size() && x.size() > 1;
}
