#pragma once
#include <cstdint>

struct DataBlock
{
    uint8_t* m_data;        // 0x00: Pointer to the binary payload
    uint64_t m_used;        // 0x08: The actual size of the payload in bytes
    uint64_t m_allocated;   // 0x10: The total allocated capacity of the buffer
    bool     m_owning;      // 0x18: Whether this instance frees the memory on destruction
};

