#pragma once
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <expected>

// Create a safe, bounds-checked copy helper function
template <size_t N>
void SafeStringCopy(char(&destination)[N], std::string_view source)
{
    size_t copyLength = (std::min)(source.length(), N - 1);

    std::copy_n(source.data(), copyLength, destination);

    // Explicitly guarantee null-termination
    destination[copyLength] = '\0';
}

// Create a safe, bounds-checked copy helper function
template <size_t N>
void SafeStringCopy(wchar_t(&destination)[N], std::wstring_view source)
{
    size_t copyLength = (std::min)(source.length(), N - 1);

    std::copy_n(source.data(), copyLength, destination);

    // Explicitly guarantee null-termination
    destination[copyLength] = L'\0';
}

std::wstring ToWString(std::string_view src);
std::string ToString(std::wstring_view src);
std::wstring GetFullModulePath(HMODULE hModule = NULL);
std::string GetErrorMessage(DWORD error);
std::expected<void, std::string> WriteStringToFile(const std::string& filepath, const std::string& content);