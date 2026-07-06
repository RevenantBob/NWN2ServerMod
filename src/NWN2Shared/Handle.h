#pragma once
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Handle
{
public:
    Handle()
        : _Handle(nullptr)
    {
    }

    Handle(HANDLE handle)
        : _Handle(handle)
    {
    }

    Handle& operator =(HANDLE handle)
    {
        _Handle = handle;

        return *this;
    }

    // Handle move operations
    Handle(Handle&& other) noexcept
    {
        _Handle = other._Handle;
        other._Handle = NULL;
    }

    Handle& operator =(Handle&& other) noexcept
    {
        _Handle = other._Handle;
        other._Handle = NULL;

        return *this;
    }

    // Disable copying
    Handle(const Handle &) = delete;
    Handle& operator =(const Handle&) = delete;


    operator HANDLE() const { return _Handle; }

    void Close()
    {
        if (_Handle)
        {
            ::CloseHandle(_Handle);
            _Handle = nullptr;
        }
    }
    // The magic
    ~Handle()
    {
        if (_Handle)
        {
            ::CloseHandle(_Handle);
        }
    }
private:
    HANDLE _Handle;
};

