#ifndef KIACONSOLE_UTIL_H
#define KIACONSOLE_UTIL_H

#include "pch.h"

#include <algorithm> 
#include <cctype>
#include <locale>
#include <vector>
#include <string>

/*! @brief Simple utility functions.

    @see trim functions from https://stackoverflow.com/a/217605
*/
class Util
{
    public:
        static std::vector<std::string> split(const std::string& s, const std::string& delim);
        static std::wstring toWstring(const char* s);
        static std::string toLower(const std::string& s);
        static std::wstring clean(const wchar_t* s);
        static void log(const wchar_t* format, ...);
        static std::string sstring(const char* format, ...);
        static std::wstring timestamp();

        static bool startswith(const std::string& s, const std::string& prefix)
        {
            return 0 == s.rfind(prefix, 0);
        }

        //! trim from start (in place)
        static inline void ltrim(std::string &s) 
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) 
            {
                return !std::isspace(ch);
            }));
        }

        //! trim from end (in place)
        static inline void rtrim(std::string &s) 
        {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) 
            {
                return !std::isspace(ch);
            }).base(), s.end());
        }

        //! trim from both ends (in place)
        static inline void trim(std::string &s) 
        {
            ltrim(s);
            rtrim(s);
        }

        //! trim from start (copying)
        static inline std::string ltrim_copy(std::string s) 
        {
            ltrim(s);
            return s;
        }

        //! trim from end (copying)
        static inline std::string rtrim_copy(std::string s) 
        {
            rtrim(s);
            return s;
        }

        //! trim from both ends (copying)
        static inline std::string trim_copy(std::string s) 
        {
            trim(s);
            return s;
        }
};

#endif
