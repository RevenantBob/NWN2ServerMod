#pragma once
#include <expected>
#include <memory>
#include <string>
#include <cinttypes>
#include <NWN2Shared.h>
#include "VirtualMachineCommands.h"
#include "DataBlock.h"
#include "CExoString.h"


typedef void(__fastcall* InitializeNetLayerFunc)(void* pThis, bool);
typedef void(__fastcall* InitializeCommandsFunc)(void* pThis);
typedef bool(__fastcall* SetBinaryDataFunc)(
    void* pThisCampaignDB,
    CExoString* pCampNameExoStr,
    CExoString* pVarNameExoStr,
    CExoString* pPlayerExoStr,
    DataBlock* pDataBlock,
    uint16_t varType);

class NWN2Mod
{
public:
    NWN2Mod(const Config config)
        : _Config(config)
        , _NWVirtualMachineCommands(nullptr)
    {
    }

    std::expected<void, uint32_t> Initialize();

    static std::expected<void, uint32_t> Initialize(std::wstring_view configPath);

    static std::unique_ptr<NWN2Mod> Current;

    template <typename... Args>
    static void Log(Logger::Level level, std::format_string<Args...> fmt, Args&&... args)
    {
        // Forward the arguments to your core log function
        Current->_Logger->log(level, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void Log(std::format_string<Args...> fmt, Args&&... args)
    {
        // Forward the arguments to your core log function
        Current->_Logger->log(Logger::Level::Info, fmt, std::forward<Args>(args)...);
    }
private:
    static InitializeNetLayerFunc _InitializeNetLayer;
    static InitializeCommandsFunc _InitializeCommands;
    static SetBinaryDataFunc _SetBinaryData;

    void MapNWNXFunctions();

    // Due to how NWN2Server64 is coded, these procs can be set directly in memory without hooking.
    static void __cdecl NWNXSetString(const char *plugin, const char *function, const char *param1, int param2, const char *value);
    static void __cdecl NWNXSetInt(const char* plugin, const char* function, const char* param1, int param2, int value);
    static void __cdecl NWNXSetFloat(const char* plugin, const char* function, const char* param1, int param2, float value);

    // Note: The returned string is NOT manipulated in any way from NWN2Server64. Therefore we can have a static buffer which will just manage
    // the result so it lasts beyond this function.
    static const char * __cdecl NWNXGetString(const char* plugin, const char* function, const char* param1, int param2);
    static int __cdecl NWNXGetInt(const char* plugin, const char* function, const char* param1, int param2);
    static float __cdecl NWNXGetFloat(const char* plugin, const char* function, const char* param1, int param2);

    static void __fastcall HookInitializeNetLayer(void *pThis, bool param);
    static void __fastcall HookInitializeCommands(void* pThis);
    static bool __fastcall HookSetBinaryData(void* pThisCampaignDB,
        CExoString* pCampNameExoStr,
        CExoString* pVarNameExoStr,
        CExoString* pPlayerExoStr,
        DataBlock* pDataBlock,
        uint16_t varType);

    static uintptr_t ExtractRipRelativeAddress(uintptr_t functionAddress,
        size_t instructionOffset,
        size_t displacementOffset,
        size_t instructionLength);

    std::expected<void, std::string> DoHooks();

    std::expected<void*, std::string> FindInitializeCommands();
    std::expected<void*, std::string> FindInitializeNetLayer();
    std::expected<void*, std::string> FindSetBinaryData();
    std::expected<void*, std::string> FindVirtualMachineWrite();
    void FinishInitialization();
    std::shared_ptr<Logger> _Logger;
    CNWVirtualMachineCommands *_NWVirtualMachineCommands;
    void *_VirtualMachine; // Needed for RunScript
    Config _Config;
};