#pragma once
#include <expected>
#include <filesystem>
#include <string>
#include <NWN2Shared.h>

class NWN2Server
{
public:
    static std::expected<void, std::string> LoadServer(std::wstring configPath);

private:
    static std::expected<PROCESS_INFORMATION, std::string> StartProcess(std::wstring processPath, std::wstring processDir, std::wstring processArgs);

    static std::expected<void *, std::string> LoadHook(HANDLE hProcess, std::wstring hookPath, Logger &log);
    static std::expected<void, std::string> LoadLoader(HANDLE hProcess, void *remoteProc, std::wstring configPath);

    static std::wstring GetLoaderDll(Config config);
};

