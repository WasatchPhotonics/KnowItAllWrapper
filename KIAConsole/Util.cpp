#include "pch.h"

#include "Util.h"

#include <sstream>
#include <cwchar>

#include <AtlBase.h>
#include <atlconv.h>
#include <time.h>
#include <stdarg.h>

using std::vector;
using std::string;
using std::wstring;
using std::stringstream;

vector<string> Util::split(const string& s, const string& delim)
{
    stringstream ss(s);
    vector<string> v;

    while (ss.good())
    {
        string tok;
        getline(ss, tok, ',');
        v.push_back(tok);
    }

    return v;
}

wstring Util::toWstring(const char* s)
{
    CA2W tmp(s);
    return wstring(tmp);
}

string Util::toLower(const string& s)
{
    string lc(s);
    for (unsigned i = 0; i < lc.size(); i++)
        lc[i] = tolower(lc[i]);
    return lc;
}

wstring Util::clean(const wchar_t* s)
{
    wstring orig(s);
    wstring dest;
    for (unsigned i = 0; i < orig.size(); i++)
        if (orig[i] != '@')
            dest += orig[i];
    return dest;
}

//! print a single timestamped log line to console with linefeed
string Util::sstring(const char* format, ...)
{
    char buf[256];

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    return string(buf);
}

wstring Util::timestamp()
{
    char buf[32] = { 0 };
    time_t now = time(NULL);
    string(ts) = ctime(&now);
    ts[ts.length() - 1] = 0;
    snprintf(buf, sizeof(buf), "%s ", ts.c_str());
    return toWstring(buf);
}


//! print a single timestamped log line to console with linefeed
void Util::log(const wchar_t* format, ...)
{
    /*
    time_t now = time(NULL);
    string(ts) = ctime(&now);
    ts[ts.length() - 1] = 0;
    printf("%s ", ts.c_str());
    */

    va_list args;
    va_start(args, format);
    vwprintf(format, args);
    va_end(args);

    printf("\n");

    fflush(stdout);
}

