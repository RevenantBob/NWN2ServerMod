#pragma once
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <expected>
#include <memory>
#include <string>
#include "Handle.h"
#include "Data.h"

class MemoryMap
{
public:
    MemoryMap()
    {
    }

    MemoryMap(Handle&& other) noexcept
        : _Handle(std::move(other))
    {
    }

    // Handle move operations
    MemoryMap(MemoryMap&& other) noexcept
    {
        _Handle = std::move(other._Handle);
    }

    MemoryMap& operator =(Handle&& other) noexcept
    {
        _Handle = std::move(other);

        return *this;
    }
    MemoryMap& operator =(MemoryMap&& other) noexcept
    {
        _Handle = std::move(other._Handle);

        return *this;
    }
    // Disable copying
    MemoryMap(const MemoryMap&) = delete;
    MemoryMap& operator =(const MemoryMap&) = delete;


    operator HANDLE() const { return _Handle; }

    std::expected<void, std::string> Create(std::wstring_view name, size_t size)
    {
        _Handle.Close();

        HANDLE hMapFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE,    // Use paging file
            NULL,                    // Default security
            PAGE_READWRITE,          // Read/write access
            (DWORD)((0xFFFFFFFF00000000 & size) >> 32ull), // Maximum object size (high-order DWORD)
            (DWORD)(0x00000000FFFFFFFF & size),       // Maximum object size (low-order DWORD)
            name.data()          // Name of mapping object
        );

        if (hMapFile == NULL)
        {
            DWORD error = ::GetLastError();
            return std::unexpected(std::format("CreateFileMappingW failed: {} (0x{:08X}", GetErrorMessage(error), error));
        }

        _Handle = hMapFile;

        return {};
    }

    std::expected<void, std::string> Open(std::wstring_view name)
    {
        _Handle.Close();

        HANDLE hMapFile = OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,   // Read access
            FALSE,           // Do not inherit the name
            name.data()  // Exact same name
        );

        if (hMapFile == NULL)
        {
            DWORD error = ::GetLastError();
            return std::unexpected(std::format("OpenFileMappingW failed: {} (0x{:08X}", GetErrorMessage(error), error));
        }

        _Handle = hMapFile;

        return {};
    }

    std::expected<void*, std::string> GetAddress(size_t offset, size_t size)
    {
        void* ptr = MapViewOfFile(
            _Handle,
            FILE_MAP_ALL_ACCESS,
            (DWORD)((0xFFFFFFFF00000000 & offset) >> 32ull),
            (DWORD)(0x00000000FFFFFFFF & offset),
            size);
        if (!ptr)
        {
            DWORD error = ::GetLastError();
            return std::unexpected(std::format("MapViewOfFile failed: {} (0x{:08X}", GetErrorMessage(error), error));
        }

        return ptr;
    }

    void UnmapAddressd(void*ptr)
    {
        UnmapViewOfFile(ptr);
    }
private:
    Handle _Handle;
};

