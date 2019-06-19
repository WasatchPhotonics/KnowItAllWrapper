#ifndef KIACONSOLE_MEASUREMENT_H
#define KIACONSOLE_MEASUREMENT_H

#include "pch.h"

#include <vector>
#include <string>
#include <fstream>

//! simple class to read and parse a spectrum from a CSV file (lines with x, y pairs)
class Measurement
{
    public:
        std::vector<double> y;
        std::vector<double> x;
        std::wstring pathname;

        Measurement(const std::wstring& pathname);
        Measurement(int pixels);

        bool isValid() const;

    private:
        void load(std::istream& infile, int pixels = 0);
        bool valid = false;
};

#endif
