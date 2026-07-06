#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <NWN2Shared.h>

#include "NWN2Mod.h"

#pragma comment(lib, "Detours.lib")

DWORD WINAPI ModInitializationThread(LPVOID lpParam)
{
    std::wstring_view path((const wchar_t*)lpParam);
    auto result = NWN2Mod::Initialize(path);
    if (!result)
    {
        return result.error();
    }

    return 0;
}

BOOL WriteAddress()
{
    MemoryMap map;

    if (!map.Open(L"Local\\NWN2Shared"))
    {
        return FALSE;
    }

    auto ptrReturn = map.GetAddress(0, 8);
    if (!ptrReturn)
    {
        return FALSE;
    }

    auto param = (size_t*)ptrReturn.value();

    *param = (size_t)&ModInitializationThread;

    map.UnmapAddressd(ptrReturn.value());

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        if (!WriteAddress())
        {
            return FALSE;
        }
    }
    return TRUE;
}