#pragma once
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "gdiplus.lib")

#define WIN32_LEAN_AND_MEAN             // Wyklucz rzadko używane rzeczy z nagłówków systemu Windows
// Pliki nagłówkowe systemu Windows
#include <windows.h>
#include <WinBase.h>
#include <stdexcept>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <iostream>
#include <shlwapi.h>
#include <comutil.h>
#include <shlobj.h>
#include <shellapi.h>
#include <gdiplus.h>

//#include "get-exe-icon.h"