#include "pch.h"

#include "Options.h"
#include "Util.h"

using std::string;

Options::Options(int argc, char** argv)
{
    // defaults
    streaming = false;
    directory = L".";

    for (int i = 1; i < argc; i++)
    {
        string s = argv[i];
        s = Util::toLower(s);

        if (s == "--streaming")
            streaming = true;
        else if (s == "--nostreaming")
            streaming = false;
        else if (s == "--directory")
        {
            if (i + 1 < argc)
            {
                i++;
                directory = Util::toWstring(argv[i]);
            }
            else
            {
                printf("ERROR: --directory requires argument\n");
                usage();
                return;
            }
        }
        else
        {
            printf("ERROR: unrecognized argument: %s\n", s.c_str());
            usage();
            return;
        }
    }
    valid = true;
}

void Options::usage()
{
    printf(
        "Bio-Rad Know-It-All Console (C) 2019, Wasatch Photonics\n\n"
        "Usage:\n"
        "  KIAConsole [--streaming] [--directory \\path\\to\\spectra]\n\n"
        "  --streaming   read series of (wavenumber, intensity) pairs from STDIN,\n"
        "                starting with 'PIXELS, n' to indicate following pixel count\n"
        "  --directory   path in which to search for .csv files (defaults to current)\n\n"
    );
}
