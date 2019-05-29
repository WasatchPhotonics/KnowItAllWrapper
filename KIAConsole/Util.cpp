#include "pch.h"

#include "Util.h"

#include <sstream>

using std::string;
using std::vector;
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
