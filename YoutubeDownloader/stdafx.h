// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifndef UNICODE
    #define UNICODE
#endif

#include <windows.h>
//#include <wininet.h>
#include <winhttp.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <vector>

// Macros
#define DELETE_POINTER(X) if(NULL!=(X)){ delete (X); (X) = NULL; }
#define DELETE_POINTER_ARRAY(X) if(NULL!=(X)){ delete[] (X); (X) = NULL; }

#define VERIFY_BOOL(X) if(TRUE != (X)){ goto cleanup;}
#define VERIFY_HRESULT(X) if(S_OK != (X)){ goto cleanup;}

#define BOOL2HR(X) ((TRUE == (X))? S_OK : E_FAIL)
#define HR2BOOL(X) ((S_OK == (X))? TRUE : FALSE)

// structure
typedef std::pair<std::wstring, std::wstring> KEY_VAL_PAIR;

// TODO: reference additional headers your program requires here
