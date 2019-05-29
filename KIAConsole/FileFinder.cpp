#include "pch.h"

#include "FileFinder.h"

#include <windows.h>

#include <stack>

using std::wstring;
using std::stack;

FileFinder::FileFinder(const wstring& directory, const wstring& mask)
{
    traverseDirectory(directory, mask);
}

void FileFinder::traverseDirectory(wstring path, const wstring& mask)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;
    wstring spec;
    stack<wstring> directories;

    directories.push(path);
    files.clear();

    while (!directories.empty())
    {
        path = directories.top();
        spec = path + L"\\" + mask;
        directories.pop();

        hFind = FindFirstFile(spec.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE) 
            return;

        do {
            if (wcscmp(ffd.cFileName, L".") != 0 &&
                wcscmp(ffd.cFileName, L"..") != 0) 
            {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
                    directories.push(path + L"\\" + ffd.cFileName);
                else 
                    files.push_back(path + L"\\" + ffd.cFileName);
            }
        } while (FindNextFile(hFind, &ffd) != 0);

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            FindClose(hFind);
            return;
        }

        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}
