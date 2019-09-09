#include "pch.h"

#include "Measurement.h"

#include "Util.h"

#include <exception>
#include <iostream>
#include <istream>
#include <fstream>

using std::string;
using std::wstring;
using std::istream;
using std::ifstream;
using std::iostream;
using std::runtime_error;

Measurement::Measurement(const wstring& pathname)
    : pathname(pathname)
{
    ifstream infile(pathname);
    load(infile);
}

Measurement::Measurement()
{
    load(std::cin);
}

void Measurement::load(istream& is)
{
    valid = false;

    char buf[256];
    int linecount = -1;
    bool using_markers = false;

    while (true)
    {
        // read the next line
        is.getline(buf, sizeof(buf));
        string line(buf);
        Util::trim(line);

        linecount++;

        // WHY do I have to convert the line to wstring before calling vwprintf?
        // Docs say %s should work with narrow string...
        //
        // Util::log(L"read line [%04d / %d]: [%s]", linecount, pixels, Util::toWstring(line.c_str()).c_str());
        
        // skip blanks and comments
        if (line.size() == 0 || line[0] == '#' || line[0] == '/')
            continue;

        // streaming data uses start/end markers to ease debugging (not required,
        // as not found in CSV files).  If we do find a start marker, expect to 
        // close on an end-marker (otherwise, stop when we've read the expected
        // number of pixels).
        if (linecount == 0 && Util::startswith(line, "REQUEST_START"))
        {
            using_markers = true;
            continue;
        }
        else if (using_markers && Util::startswith(line, "REQUEST_END"))
        {
            break;
        }
        else if (Util::startswith(line, "QUIT"))
        {
            Util::log(L"QUIT received...shutting down");
            isQuit = true;
            return;
        }

        // Other than unary tokens above, subsequent data is presumed to be comma-
        // delimited and contain at least two fields.
        std::vector<string> tokens = Util::split(line, ",");
        if (tokens.size() < 2)
            throw runtime_error(Util::sstring("invalid number of tokens: %d (%s)", tokens.size(), line.c_str()));

        // parse supported request metadata
        string field = Util::toLower(tokens[0]);
        if (field == "pixels" || field == "pixel count")
        {
            pixels = atoi(tokens[1].c_str());
            continue;
        }
        else if (field == "max_results")
        {
            max_results = atoi(tokens[1].c_str());
            continue;
        }
        else if (field == "min_confidence")
        {
            // expecting value in range (0, 100) (not 0.00 to 1.00)
            min_confidence = stod(tokens[1]);
            continue;
        }

        // that's all the metadata we support, so otherwise skip lines that don't
        // start with a digit (lets us parse standard ENLIGHTEN column-ordered CSV)
        if (!(('0' <= line[0] && line[0] <= '9') || line[0] == '-'))
            continue;

        // presumably we're now reading pixel data
        x.push_back(stod(tokens[0]));
        y.push_back(stod(tokens[1]));

        if (!using_markers && x.size() == pixels)
        {
            Util::log(L"Read expected %d pixels", pixels); // matched by KIAWrapper
            break;
        }
    }

    Util::log(L"Finished loading measurement");
    if (pixels == x.size())
    {
        Util::log(L"Measurement valid (found expected %d pixels)", pixels);
        valid = true;
    }
    else
    {
        Util::log(L"Measurement invalid; read %d of %d pixels", x.size(), pixels);
    }
}

bool Measurement::isValid() const
{
    return valid && x.size() == y.size() && x.size() > 1;
}
