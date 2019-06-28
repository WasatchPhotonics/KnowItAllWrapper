#ifndef KIACONSOLE_MEASUREMENT_H
#define KIACONSOLE_MEASUREMENT_H

#include "pch.h"

#include <vector>
#include <string>
#include <istream>

//! Represents a spectral measurement which KIAConsole is asked to identify.
//! In enlighten.KIAWrapper, this would correspond to a KIARequest.
class Measurement
{
    public:
        int pixels = 1024;
        int max_results = 20;
        double min_confidence = 0.60;
        std::vector<double> y;
        std::vector<double> x;

        std::wstring pathname;                      //!< if loaded from an external file, vs streaming


        Measurement(const std::wstring& pathname);  //!< instantiate from an external file
        Measurement();                              //!< stream from stdin

        bool isValid() const;
        bool isQuit = false;

    private:
        void load(std::istream& infile);
        bool valid = false;
};

#endif
