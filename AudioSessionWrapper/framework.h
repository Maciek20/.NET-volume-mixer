#pragma once
#pragma comment(lib, "comsuppw.lib")

#define WIN32_LEAN_AND_MEAN             // Wyklucz rzadko używane rzeczy z nagłówków systemu Windows
// Pliki nagłówkowe systemu Windows
#include <windows.h>
#include <stdexcept>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <wrl/client.h>
#include <iostream>
#include <shlwapi.h>
#include <comutil.h>