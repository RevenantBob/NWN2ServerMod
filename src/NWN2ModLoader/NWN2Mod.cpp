#include "NWN2Mod.h"
#include "PEPattern.h"
#include <detours.h>
#include "Commands.h"

std::unique_ptr<NWN2Mod> NWN2Mod::Current;

InitializeNetLayerFunc NWN2Mod::_InitializeNetLayer;
InitializeCommandsFunc NWN2Mod::_InitializeCommands;
SetBinaryDataFunc NWN2Mod::_SetBinaryData;

std::expected<void, uint32_t> NWN2Mod::Initialize()
{
    _Logger = std::make_shared<Logger>(
        _Config.loader_log.transform([](auto v) { return::ToWString(v); }),
        _Config.std_log.value_or(true),
        _Config.debug_log.value_or(false));

    Log("*** Initializing NWN2 Hooks ***");

    auto hooksResult = DoHooks();
    if (!hooksResult)
    {
        Log("Failed to hook functions: {}", hooksResult.error());

        return std::unexpected(ERROR_HOOK_NOT_INSTALLED);
    }

    Log("Initializing complete.");

    return {};
}

std::expected<void, uint32_t> NWN2Mod::Initialize(std::wstring_view configPath)
{
    auto configResult = Config::FromFile(configPath);
    if (!configResult)
    {
        // Can't log this error since we have no config loaded.
        return std::unexpected(ERROR_INVALID_PARAMETER);
    }

    auto config = *configResult;

    Current = std::make_unique<NWN2Mod>(config);
    return Current->Initialize();
}
std::expected<void, std::string> NWN2Mod::DoHooks()
{
    auto initCommands = FindInitializeCommands();
    if (!initCommands)
    {
        return std::unexpected(std::format("Failed to find InitializeCommands: {}", initCommands.error()));
    }

    auto initNet = FindInitializeNetLayer();
    if (!initNet)
    {
        return std::unexpected(std::format("Failed to find InitializeNetLayer: {}", initNet.error()));
    }

    auto virtualMachine = FindVirtualMachineWrite();
    if (!virtualMachine)
    {
        return std::unexpected(std::format("Failed to find g_pVirtualMachine: {}", virtualMachine.error()));
    }

    auto setBinaryData = FindSetBinaryData();
    if (!setBinaryData)
    {
        return std::unexpected(std::format("Failed to find SetBinaryData: {}", setBinaryData.error()));
    }

    // Sanity check
    uint8_t *pBuffer = (uint8_t *)initNet.value();
    if (pBuffer[0] != 0x48)
    {
        return std::unexpected(std::format("Found invalid address for InitializeNetLayer: 0x{:016X} Data:0x{:02X}", (uint64_t)pBuffer, *pBuffer));
    }

    pBuffer = (uint8_t*)initCommands.value();
    if (pBuffer[0] != 0x48)
    {
        return std::unexpected(std::format("Found invalid address for InitializeCommands: 0x{:016X} Data:0x{:02X}", (uint64_t)pBuffer, *pBuffer));
    }

    pBuffer = (uint8_t*)setBinaryData.value();
    if (pBuffer[0] != 0x48)
    {
        return std::unexpected(std::format("Found invalid address for SetBinaryData: 0x{:016X} Data:0x{:02X}", (uint64_t)pBuffer, *pBuffer));
    }
    // Should be initialized to 0x0000000000000000
    if (*((uintptr_t*)virtualMachine.value()) != 0x0000000000000000)
    {
        return std::unexpected(std::format("Found invalid address for g_pVirtualMachine: 0x{:016X} Data:0x{:016X}", ((uint64_t)virtualMachine.value()), *((uint64_t*)virtualMachine.value())));
    }

    _InitializeCommands = (InitializeCommandsFunc)initCommands.value();
    _InitializeNetLayer = (InitializeNetLayerFunc)initNet.value();
    _SetBinaryData = (SetBinaryDataFunc)setBinaryData.value();

    _VirtualMachine = virtualMachine.value(); // This is the address of the global. We'll fix it to the value of the global later. It lasts until server shutdown.

    // Populate the static variables with the hex addresses you found
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)_InitializeNetLayer, &HookInitializeNetLayer);
    DetourAttach(&(PVOID&)_InitializeCommands, &HookInitializeCommands);
    DetourAttach(&(PVOID&)_SetBinaryData, &HookSetBinaryData);

    LONG error = DetourTransactionCommit();
    if (error != NO_ERROR)
    {
        // Return the error state if Detours fails
        return std::unexpected(std::format("Detours failed to attach. Error code: {}", error));
    }

    return {};
}

void __cdecl NWN2Mod::NWNXSetString(const char* plugin, const char* function, const char* param1, int param2, const char* value)
{
    std::string message = std::format("NNXSetString(\"{}\", \"{}\", \"{}\", {}, \"{}\")", plugin, function, param1, param2, value);

    ::MessageBoxA(NULL, message.c_str(), "NWN2Mod", MB_OK);
}

void __cdecl NWN2Mod::NWNXSetInt(const char* plugin, const char* function, const char* param1, int param2, int value)
{
    std::string message = std::format("NWNXSetInt(\"{}\", \"{}\", \"{}\", {}, {})", plugin, function, param1, param2, value);

    ::MessageBoxA(NULL, message.c_str(), "NWN2Mod", MB_OK);
}

void __cdecl NWN2Mod::NWNXSetFloat(const char* plugin, const char* function, const char* param1, int param2, float value)
{
    std::string message = std::format("NWNXSetFloat(\"{}\", \"{}\", \"{}\", {}, {})", plugin, function, param1, param2, value);

    ::MessageBoxA(NULL, message.c_str(), "NWN2Mod", MB_OK);
}

const char *__cdecl NWN2Mod::NWNXGetString(const char* plugin, const char* function, const char* param1, int param2)
{
    std::string message = std::format("NWNXGetString(\"{}\", \"{}\", \"{}\", {})", plugin, function, param1, param2);

    ::MessageBoxA(NULL, message.c_str(), "NWN2Mod", MB_OK);

    return "";
}

int __cdecl NWN2Mod::NWNXGetInt(const char* plugin, const char* function, const char* param1, int param2)
{
    std::string message = std::format("NWNXGetInt(\"{}\", \"{}\", \"{}\", {})", plugin, function, param1, param2);

    ::MessageBoxA(NULL, message.c_str(), "NWN2Mod", MB_OK);

    return 0;
}

float __cdecl NWN2Mod::NWNXGetFloat(const char* plugin, const char* function, const char* param1, int param2)
{
    std::string message = std::format("NWNXGetFloat(\"{}\", \"{}\", \"{}\", {})", plugin, function, param1, param2);

    ::MessageBoxA(NULL, message.c_str(), "NWN2Mod", MB_OK);

    return 0.0f;
}

void __fastcall NWN2Mod::HookInitializeNetLayer(void* pThis, bool param)
{
    NWN2Mod::Log("InitializeNetLayer called.");

    // Here it's safe to initialize all the modules where they can read the commands
    Current->FinishInitialization();

    _InitializeNetLayer(pThis, param);
}

void __fastcall NWN2Mod::HookInitializeCommands(void* pThis)
{
    NWN2Mod::Log("InitializeCommands called.");

    // Save off "this" as it's a global variable we can use to access the commands later.
    NWN2Mod::Current->_NWVirtualMachineCommands = (CNWVirtualMachineCommands *)pThis;

    _InitializeCommands(pThis);
}

bool __fastcall NWN2Mod::HookSetBinaryData(
    void* pThisCampaignDB,
    CExoString* pCampNameExoStr,
    CExoString* pVarNameExoStr,
    CExoString* pPlayerExoStr,
    DataBlock* pDataBlock,
    uint16_t varType)
{
    NWN2Mod::Log("SetBinaryData called.");

    return true;
}

std::expected<void*, std::string> NWN2Mod::FindInitializeCommands()
{
    // std::string sig = "48 89 10 48 89 50 08 49 8D 50 ?? 48 8B 02 48 8D 0D ?? ?? ?? ?? 48 89 08 48 8B 02 48 8D 0D ?? ?? ?? ?? 48 89 48 08";
    // This pattern matches this opcode pattern here:
    // 1407530f9 48 89 10        MOV        qword ptr [RAX],RDX
    // 1407530fc 48 89 50 08     MOV        qword ptr[RAX + 0x8], RDX
    // 140753100 49 8d 50 18     LEA        RDX, [R8 + 0x18]
    // 140753104 48 8b 02        MOV        RAX, qword ptr[RDX]
    // 140753107 48 8d 0d        LEA        this, [CNWVirtualMachineCommands::ExecuteComman
    // 22 7a 00 00
    // 14075310e 48 89 08        MOV        qword ptr[RAX], this = > CNWVirtualMachineCommand
    // 140753111 48 8b 02        MOV        RAX, qword ptr[RDX]
    // 140753114 48 8d 0d        LEA        this, [CNWVirtualMachineCommands::ExecuteComman
    // a5 7a 00 00
    // 14075311b 48 89 48 08     MOV        qword ptr[RAX + 0x8], this = > CNWVirtualMachineC

    // The idea is, this is a unique area of code where the entire m_pVirtualMachineCommands array is initialized. Finding this location
    // gives us access to ALOT of the virtual machine and functions in the NWN2Server64.exe. For one, the table is a pointer to every
    // script function that can be called, but it only gets initialized programmatically after the server is running.
    // This entire section of code has the function pointers, so we COULD read them all here, but that would be a lot of iterating for
    // little gain. Better to save it's pointer.


    // This sig is for the START of the function.
    std::string sig = "48 89 5C 24 20 48 89 4C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC B0 00 00 00 48 8B C1 33 C9 88 48 10 88 0D ?? ?? ?? ?? 88 0D ?? ?? ?? ??";

    return PEPattern::FindPattern(L"NWN2Server64.exe", sig);
}

std::expected<void*, std::string> NWN2Mod::FindInitializeNetLayer()
{
    // This is the byte pattern for the beginning of the CServerExoAppInternal::InitializeNetLayer. This is called
    // right after the InitializeCommands. So this is a safe place to read the m_pVirtualMachineCommands member with all the
    // functions in it.
    
    std::string pattern = "48 89 5C 24 20 88 54 24 10 48 89 4C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D9 48 81 EC 00 01 00 00 33 C0 48 89 45 8F 89 45 97 41 B8 F9 01 00 00 48 8D 15 ?? ?? ?? ??";
    return PEPattern::FindPattern(L"NWN2Server64.exe", pattern);
}

std::expected<void*, std::string> NWN2Mod::FindSetBinaryData()
{
    // This is the byte pattern for the beginning of the CCampaignDB::SetBinaryData. This is hooked so we can store
    // binary objects how we see fit and restore them.

    std::string pattern = "48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ?? 48 81 EC ?? ?? ?? ?? 4D 8B E1 49 8B F8 48 8B F2 4C 8B E9 48 8D 4D ?? E8 ?? ?? ?? ??";
    return PEPattern::FindPattern(L"NWN2Server64.exe", pattern);
}



std::expected<void*, std::string> NWN2Mod::FindVirtualMachineWrite()
{
    // This is the byte pattern for a section of code that writes the m_pVirtualMachine in StartServices.
    std::string pattern = "41 B8 32 12 00 00 48 8D 15 ?? ?? ?? ?? B9 08 05 00 00 FF 15 ?? ?? ?? ?? 48 89 85 ?? ?? ?? ?? 48 85 C0 74 11 49 8B 95 ?? ?? ?? ?? 48 8B C8 E8 ?? ?? ?? ?? EB 03 49 8B C4 48 89 05 ?? ?? ?? ??";

    auto result = PEPattern::FindPattern(L"NWN2Server64.exe", pattern);
    if (!result)
    {
        return std::unexpected(result.error());
    }

    uintptr_t offset = (uintptr_t)result.value();

    // The instruction you want starts 0x38 bytes ahead:
    // 140570285: 48 89 05 5C C0 2D 01
    offset += 0x38;

    auto absoluteAddress = ExtractRipRelativeAddress(offset, 0, 3, 7);
    
    return (void *)absoluteAddress;
}

uintptr_t  NWN2Mod::ExtractRipRelativeAddress(uintptr_t functionAddress,
    size_t instructionOffset,
    size_t displacementOffset,
    size_t instructionLength)
{
    // 1. Calculate the absolute address of the *next* instruction (the RIP base used by the CPU)
    uintptr_t nextInstructionRIP = functionAddress + instructionOffset + instructionLength;

    // 2. Read the 4-byte signed displacement offset from the function memory space
    int32_t relativeDisplacement = 0;
    std::memcpy(&relativeDisplacement, reinterpret_cast<void*>(functionAddress + displacementOffset), sizeof(int32_t));

    // 3. The absolute address is Next RIP + Signed Displacement
    return nextInstructionRIP + relativeDisplacement;
}

void NWN2Mod::MapNWNXFunctions()
{
    struct Func
    {
        size_t functionId;
        size_t instructionOffset;
        size_t displacementOffset;
        size_t opcodeSize;
        void *function;
    };

    Func funcs[] = {
        { Commands::COMMAND_NWNXSETSTRING,  0x3AC,  0x3AF,  7,  (void*)&NWN2Mod::NWNXSetString },
        { Commands::COMMAND_NWNXSETINT,     0x305,  0x308,  7,  (void*)&NWN2Mod::NWNXSetInt },
        { Commands::COMMAND_NWNXSETFLOAT,   0x305,  0x308,  7,  (void*)&NWN2Mod::NWNXSetFloat },
        { Commands::COMMAND_NWNXGETSTRING,  0x2C0,  0x2C3,  7,  (void*)&NWN2Mod::NWNXGetString },
        { Commands::COMMAND_NWNXGETINT,     0x2B8,  0x2BB,  7,  (void*)&NWN2Mod::NWNXGetInt },
        { Commands::COMMAND_NWNXGETFLOAT,   0x2B8,  0x2BB,  7,  (void*)&NWN2Mod::NWNXGetFloat },
    };

    for (size_t i = 0; i < 6; ++i)
    {
        auto functionAddress = (uintptr_t)_NWVirtualMachineCommands->m_pVirtualMachineCommands[funcs[i].functionId];

        // Distance calculations from function start (0x1407bbda0) to opcode locations
        uintptr_t hookAddress = ExtractRipRelativeAddress(
            functionAddress,
            funcs[i].instructionOffset, // 1407bc14c - 1407bbda0
            funcs[i].displacementOffset,// 1407bc14f - 1407bbda0
            funcs[i].opcodeSize
        );

        void** ppHookAddress = reinterpret_cast<void**>(hookAddress);


        if (((uintptr_t)*ppHookAddress) != 0)
        {
            _Logger->log("Invalid address found.");
        }
        else
        {
            *ppHookAddress = funcs[i].function;
        }
    }
}
void NWN2Mod::FinishInitialization()
{
    MapNWNXFunctions();

    // Fixup Virtual Machine
    _VirtualMachine = *((void **)_VirtualMachine);
}