#ifndef KIACONSOLE_FILE_FINDER_H
#define KIACONSOLE_FILE_FINDER_H

#include "pch.h"

#include <string>
#include <list>

/*! @brief A simple class to traverse a directory tree and generate a list of 
           files matching a given glob pattern.

    @see https://stackoverflow.com/a/67336
*/
class FileFinder
{
    public:
        std::list<std::wstring> directories;
        std::list<std::wstring> files;

        //! @param directory  path to traverse
        //! @param mask       glob pattern, e.g. "*.csv"
        FileFinder(const std::wstring& directory, const std::wstring& mask);

    private:
        void traverseDirectory(std::wstring path, const std::wstring& mask);
};

#endif
