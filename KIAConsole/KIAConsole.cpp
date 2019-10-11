#include "pch.h"

#include <atlconv.h>        // for LPOLESTR, etc
#include <atlstr.h>         // for CString
#include <winbase.h>        // for SetDllDirectory?

#include "SearchSDK.h"      // Bio-Rad API

#include "FileFinder.h"
#include "Measurement.h"
#include "Options.h"
#include "Util.h"

#include <list>
#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>

using std::list;
using std::vector;
using std::string;
using std::chrono::system_clock;
using std::chrono::duration;
using std::wstring;

static wstring VERSION = L"0.5.1";

// function pointers into the SearchSDK DLL
static SearchSDK_InitFn                     s_SearchSDK_InitFn;
static SearchSDK_ExitFn                     s_SearchSDK_ExitFn;
static SearchSDK_OpenSearchFn               s_SearchSDK_OpenSearchFn;
static SearchSDK_CloseSearchFn              s_SearchSDK_CloseSearchFn;
static SearchSDK_RunSearchEvenlySpacedFn    s_SearchSDK_RunSearchEvenlySpacedFn;
static SearchSDK_RunSearchUnevenlySpacedFn  s_SearchSDK_RunSearchUnevenlySpacedFn;
static SearchSDK_CancelSearchFn             s_SearchSDK_CancelSearchFn;
static SearchSDK_GetProgressPercentageFn    s_SearchSDK_GetProgressPercentageFn;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                              Lifecycle                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

//! given a loaded DLL, grab key function handles
bool mapFunctionHandles(HMODULE hModule)
{
    Util::log(L"Obtaining function handles");
    s_SearchSDK_InitFn                    = reinterpret_cast<SearchSDK_InitFn                   >(::GetProcAddress(hModule, "SearchSDK_Init"));
    s_SearchSDK_ExitFn                    = reinterpret_cast<SearchSDK_ExitFn                   >(::GetProcAddress(hModule, "SearchSDK_Exit"));
    s_SearchSDK_OpenSearchFn              = reinterpret_cast<SearchSDK_OpenSearchFn             >(::GetProcAddress(hModule, "SearchSDK_OpenSearch"));
    s_SearchSDK_CloseSearchFn             = reinterpret_cast<SearchSDK_CloseSearchFn            >(::GetProcAddress(hModule, "SearchSDK_CloseSearch"));
    s_SearchSDK_RunSearchEvenlySpacedFn   = reinterpret_cast<SearchSDK_RunSearchEvenlySpacedFn  >(::GetProcAddress(hModule, "SearchSDK_RunSearchEvenlySpaced"));
    s_SearchSDK_RunSearchUnevenlySpacedFn = reinterpret_cast<SearchSDK_RunSearchUnevenlySpacedFn>(::GetProcAddress(hModule, "SearchSDK_RunSearchUnevenlySpaced"));
    s_SearchSDK_CancelSearchFn            = reinterpret_cast<SearchSDK_CancelSearchFn           >(::GetProcAddress(hModule, "SearchSDK_CancelSearch"));
    s_SearchSDK_GetProgressPercentageFn   = reinterpret_cast<SearchSDK_GetProgressPercentageFn  >(::GetProcAddress(hModule, "SearchSDK_GetProgressPercentage"));

    if (!s_SearchSDK_InitFn                    ||
        !s_SearchSDK_ExitFn                    ||
        !s_SearchSDK_OpenSearchFn              ||
        !s_SearchSDK_CloseSearchFn             ||
        !s_SearchSDK_RunSearchEvenlySpacedFn   ||
        !s_SearchSDK_RunSearchUnevenlySpacedFn ||
        !s_SearchSDK_CancelSearchFn            ||
        !s_SearchSDK_GetProgressPercentageFn)
    {
        Util::log(L"ERROR: could not obtain one or more search handles");
        return false;
    }
    return true;
}

//! load the SearchSDK.DLL
bool loadDLL()
{
    static const GUID clsid = {0xd8711b25, 0x71ca, 0x11d3, {0x9d, 0xfd, 0x0, 0xe0, 0x81, 0x10, 0x22, 0x90}};
    LPOLESTR lpOleStr = 0;
    ::StringFromCLSID(clsid, &lpOleStr);
    if (!lpOleStr || !lpOleStr[0])
    {
        Util::log(L"ERROR: could not generate Class GUID");
        return false;
    }
    Util::log(L"Generated Class GUID %ls", lpOleStr);

    WCHAR sToFind[256];
    swprintf_s(sToFind, L"CLSID\\%s\\LocalServer32", lpOleStr);
    ::CoTaskMemFree(lpOleStr);

    Util::log(L"Looking for registry key: %ls", sToFind);
    HKEY hKey=0;
    ::RegOpenKey(HKEY_CLASSES_ROOT, sToFind, &hKey);
    if (!hKey)
    {
        Util::log(L"ERROR: could not find registry key: %ls", sToFind);
        return false;
    }

    WCHAR filePath[_MAX_PATH+12] = {0}; 
    LONG len = _MAX_PATH;
    Util::log(L"Querying for filepath associated with registry key");
    ::RegQueryValue(hKey, L"", filePath, &len);
    ::RegCloseKey(hKey);
    if (!filePath[0])
    {
        Util::log(L"ERROR: could not find filepath associated with registry key");
        return false;
    }

    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR+12];
    _wsplitpath(filePath, drive, dir, 0, 0);
    _wmakepath(filePath, drive, dir, 0, 0);

    // In order for the SearchSDK.dll to load all of its dependencies, it is necessary to set the DLL directory
    Util::log(L"Setting DDL search path to %ls", filePath);
    SetDllDirectory(filePath); 

    wcscat(filePath, L"SearchSDK");
    wcscat(filePath, L".dll");

    Util::log(L"Trying to load %ls", filePath);
    HMODULE hModule = ::LoadLibrary(filePath);
    if (!hModule)
    {
        // Try to load the 32 bit version, if available.

        // move search dir to 32-bit dir
        wcscat(dir, L"32 bit DLLs\\");
        _wmakepath(filePath, drive, dir, 0, 0);
        SetDllDirectory(filePath); 

        // load 32-bit DLL
        _wmakepath(filePath, drive, dir, L"SearchSDK", L".dll");
        Util::log(L"Trying to load %ls", filePath);
        hModule = ::LoadLibrary(filePath);
        if (!hModule)
        {
            Util::log(L"ERROR: could not find or load SearchSDK.dll");
            return false;
        }
    }

    // Take handles to DLL function entry points
    return mapFunctionHandles(hModule);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               Processing                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

bool processMeasurement(const Measurement& m)
{
    auto start = system_clock::now();

    Util::log(L"Begin processing");
    SEARCHSDK_HANDLE hSearch = s_SearchSDK_OpenSearchFn();
    if (NULL == hSearch)
        return false;

    // allocate storage for matches
    SearchSDK_Match* matches = new SearchSDK_Match[m.max_results];
    int matchCount = m.max_results;

    Util::log(L"Calling RunSearchUnevenlySpaced");
    s_SearchSDK_RunSearchUnevenlySpacedFn(
        hSearch, 
        SEARCHSDK_TECHNIQUE_RAMAN,
       &m.x[0],
       &m.y[0],
        (int) m.x.size(),
        SEARCHSDK_XUNIT_WAVENUMBERS,
        SEARCHSDK_YUNIT_ARBITRARYINTENSITY,
        matches, 
       &matchCount);

    auto end = system_clock::now();
    duration<double> elapsedSec = end - start;

    // count matches that meet the threshold
    int validCount = 0;
    for (int i = 0; i < matchCount; i++)
    {
        SearchSDK_Match& match = matches[i];

        // skip low-quality matches
        if (match.m_matchPercentage >= m.min_confidence)
            validCount++;
    }

    Util::log(L"Found %d matches in %0.2lf sec", validCount, elapsedSec); // matched by KIAWrapper
    for (int i = 0; i < matchCount; i++)
    {
        SearchSDK_Match& match = matches[i];

        // skip low-quality matches
        if (match.m_matchPercentage < m.min_confidence)
            continue;

        Util::log(L"Match %d: %ls with %.2lf%% confidence (%ls)", // matched by KIAWrapper
            i, match.m_matchName, 100.0 * match.m_matchPercentage, match.m_bLocked ? L"expired" : L"licensed");
    }

    // releases any resources associated with this searchinfile
    s_SearchSDK_CloseSearchFn(hSearch);
    delete[] matches;

    Util::log(L"Processing complete");
    return true;
}

//! Process and match a single CSV spectrum
//! @param pathname path to the CSV
void processFile(const wstring& pathname)
{
    // load the file
    Util::log(L"Loading %ls", pathname.c_str());
    Measurement m(pathname);
    if (!m.isValid())
        return;

    if (!processMeasurement(m))
        Util::log(L"ERROR: could not open search on %ls", pathname.c_str());
}

void processDirectory(const Options& opts)
{
    Util::log(L"Searching for CSV files in %s", opts.directory.c_str());

    FileFinder ff(opts.directory, L"*.csv");
    Util::log(L"Found %u files", (unsigned)ff.files.size());
    ff.files.sort();

    // process each matching file
    for (list<wstring>::const_iterator file_iter = ff.files.begin(); file_iter != ff.files.end(); file_iter++)
    {
        const wstring& pathname = *file_iter;
        Util::log(L"Processing %ls", pathname.c_str());
        processFile(pathname);
    }
}

void processStream(const Options& opts)
{
    Util::log(L"Starting stream processing");

    while (true)
    {
        try
        {
            // not sure, but suspect this is actually throwing an EOF exception 
            // we're not catching on shutdown
            Measurement m;
            if (m.isQuit)
                break;
            processMeasurement(m);
        }
        catch (std::exception &e)
        {
            Util::log(L"ERROR: exception parsing streamed input: %s", e.what()); 
            break;
        }
    }
    Util::log(L"Stream processing complete");
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  Main                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    Util::log(L"KIAConsole version %ls", VERSION.c_str());

    // parse command-line arguments
    Options opts(argc, argv);
    if (!opts.valid)
        return -1;

    // load Bio-Rad's SearchSDK.dll
    if (!loadDLL())
        return -1;

    // Initialize the DLL
    Util::log(L"Initializing library");
    s_SearchSDK_InitFn();

    // Process spectra
    if (opts.streaming)
        processStream(opts);
    else
        processDirectory(opts);

    // Shutdown
    Util::log(L"Closing library");
    s_SearchSDK_ExitFn();

    Util::log(L"KIAConsole exiting");
    return 0;
}
