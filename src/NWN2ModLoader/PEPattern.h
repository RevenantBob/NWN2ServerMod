#pragma once
#include <expected>
#include <string>
#include <vector>

class PEPattern
{
public:
    // Usage Example inside your DLL initialization thread:
    static std::expected<void*, std::string> ExtractCommandTableOffset(std::wstring_view module, std::string_view pattern);

    // The core pattern scanner
    static std::expected<void*, std::string> FindPattern(std::wstring_view moduleName, std::string_view pattern);
    
    // Pattern scan with range max
    static std::expected<void*, std::string> FindPatternRange(std::wstring_view moduleName, DWORD offset, DWORD max, std::string_view pattern);

    // Helper to convert IDA-style hex string to byte/mask vectors
    static std::expected<std::vector<int>, std::string> ParseSignature(std::string_view signature);
private:
    template <typename T>
    static T ToHexValue(char c)
    {
        if (c >= 'A' && c <= 'F')
        {
            return ((T)0xA) + ((T)(c - 'A'));
        }
        else if (c >= 'a' && c <= 'f')
        {
            return ((T)0xA) + ((T)(c - 'a'));
        }
        else if (c >= '0' && c <= '9')
        {
            return ((T)(c - '0'));
        }

        return (T)0;
    }





};

