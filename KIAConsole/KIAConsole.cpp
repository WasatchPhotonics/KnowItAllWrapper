#include "pch.h"

#include <atlconv.h>        // for LPOLESTR, etc
#include <atlstr.h>         // for CString
#include <winbase.h>        // for SetDllDirectory?

#include "SearchSDK.h"      // Bio-Rad API

#include "FileFinder.h"
#include "Measurement.h"

#include <list>
#include <string>
#include <vector>
#include <stdexcept>

using std::list;
using std::vector;
using std::string;
using std::wstring;

// function pointers into the SearchSDK DLL
static SearchSDK_InitFn                     s_SearchSDK_InitFn;
static SearchSDK_ExitFn                     s_SearchSDK_ExitFn;
static SearchSDK_OpenSearchFn               s_SearchSDK_OpenSearchFn;
static SearchSDK_CloseSearchFn              s_SearchSDK_CloseSearchFn;
static SearchSDK_RunSearchEvenlySpacedFn    s_SearchSDK_RunSearchEvenlySpacedFn;
static SearchSDK_RunSearchUnevenlySpacedFn  s_SearchSDK_RunSearchUnevenlySpacedFn;
static SearchSDK_CancelSearchFn             s_SearchSDK_CancelSearchFn;
static SearchSDK_GetProgressPercentageFn    s_SearchSDK_GetProgressPercentageFn;

//! Process and match a single CSV spectrum
//! @param pathname path to the CSV
void processFile(const wstring& pathname)
{
    // load the file
    printf("loading %ls\n", pathname.c_str());
    Measurement measurement(pathname);
    if (!measurement.isValid())
        return;

    printf("  calling SearchSDK...\n");
    SEARCHSDK_HANDLE hSearch = s_SearchSDK_OpenSearchFn();
    REQUIRE(hSearch);

    // support up to 50 matches
    const int MAX_MATCHES = 50;
    SearchSDK_Match matches[MAX_MATCHES];
    int matchCount = MAX_MATCHES;

    REQUIRE(s_SearchSDK_RunSearchUnevenlySpacedFn(
        hSearch, 
        SEARCHSDK_TECHNIQUE_RAMAN,
       &measurement.x[0],
       &measurement.y[0],
        (int) measurement.x.size(),
        SEARCHSDK_XUNIT_WAVENUMBERS,
        SEARCHSDK_YUNIT_ARBITRARYINTENSITY,
        matches, 
       &matchCount));

    printf("  %d matches found\n", matchCount);
    for (int i = 0; i < matchCount; i++)
    {
        SearchSDK_Match& match = matches[i];

        printf("    Match %2d: %ls (%.2lf%% confidence)\n", i, match.m_matchName, match.m_matchPercentage);
    }

    // releases any resources associated with this search
    s_SearchSDK_CloseSearchFn(hSearch);
}

//! given a loaded DLL, grab key function handles
bool mapFunctionHandles(HMODULE hModule)
{
    printf("Obtaining function handles\n");
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
        printf("ERROR: could not obtain one or more search handles\n");
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
        printf("ERROR: could not generate Class GUID\n");
        return false;
    }
    printf("Generated Class GUID %ls", lpOleStr);

    WCHAR sToFind[256];
    swprintf_s(sToFind, L"CLSID\\%s\\LocalServer32", lpOleStr);
    ::CoTaskMemFree(lpOleStr);

    printf("Looking for registry key: %ls\n", sToFind);
    HKEY hKey=0;
    ::RegOpenKey(HKEY_CLASSES_ROOT, sToFind, &hKey);
    if (!hKey)
    {
        printf("ERROR: could not find registry key: %ls\n", sToFind);
        return false;
    }

    WCHAR filePath[_MAX_PATH+12] = {0}; 
    LONG len = _MAX_PATH;
    printf("Querying for filepath associated with registry key\n");
    ::RegQueryValue(hKey, L"", filePath, &len);
    ::RegCloseKey(hKey);
    if (!filePath[0])
    {
        printf("ERROR: could not find filepath associated with registry key\n");
        return false;
    }

    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR+12];
    _wsplitpath(filePath, drive, dir, 0, 0);
    _wmakepath(filePath, drive, dir, 0, 0);

    // In order for the SearchSDK.dll to load all of its dependencies, it is necessary to set the DLL directory
    printf("Setting DDL search path to %ls\n", filePath);
    SetDllDirectory(filePath); 

    wcscat(filePath, L"SearchSDK");
    wcscat(filePath, L".dll");

    printf("Trying to load %ls\n", filePath);
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
        printf("Trying to load %ls\n", filePath);
        hModule = ::LoadLibrary(filePath);
        if (!hModule)
        {
            printf("ERROR: could not find or load SearchSDK.dll\n");
            return false;
        }
    }

    // Take handles to DLL function entry points
    return mapFunctionHandles(hModule);
}

int main(int argc, char** argv)
{
    printf("KIAConsole starting\n");

    // load Bio-Rad's SearchSDK.dll
    if (!loadDLL())
        return -1;

    // Initialize the DLL
    printf("Initializing library\n");
    s_SearchSDK_InitFn();

    // generate list of CSV files in current directory
    printf("Searching for CSV files...");
    FileFinder ff(L".", L"*.csv");
    printf("found %u files\n", (unsigned) ff.files.size());

    // process each matching file
    for (list<wstring>::const_iterator file_iter = ff.files.begin(); file_iter != ff.files.end(); file_iter++)
    {
        const wstring& pathname = *file_iter;
        printf("processing %ls\n", pathname.c_str());
        processFile(pathname);
    }

    // Shutdown
    printf("Closing library\n");
    s_SearchSDK_ExitFn();

    printf("KIAConsole exiting\n");
    return 0;
}
