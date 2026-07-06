#pragma once
#include <cinttypes>
#include <cstddef>

class CNWVirtualMachineCommands
{
public:
    // The Virtual function forces the 8-byte VTable pointer at 0x000
    virtual ~CNWVirtualMachineCommands() = default;

    // The Variables (Compiler automatically pads the gaps)
    bool m_bValidObjectRunScript;           // 0x008
    uint32_t m_oidObjectRunScript;          // 0x00C
    bool m_bRunscriptNotificationEnabled;   // 0x010
    void** m_pVirtualMachineCommands;       // 0x018
};

static_assert(offsetof(CNWVirtualMachineCommands, m_bValidObjectRunScript) == 0x008, "Offset mismatch!");
static_assert(offsetof(CNWVirtualMachineCommands, m_oidObjectRunScript) == 0x00C, "Offset mismatch!");
static_assert(offsetof(CNWVirtualMachineCommands, m_bRunscriptNotificationEnabled) == 0x010, "Offset mismatch!");
static_assert(offsetof(CNWVirtualMachineCommands, m_pVirtualMachineCommands) == 0x018, "Offset mismatch!");