
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <iostream>
#include <string>
#include <filesystem>
#include "NWN2ServerModInfo.h"

#include <NWN2Shared.h>
#include "NWN2Server.h"

std::vector<std::wstring> GetArguments()
{
    std::vector<std::wstring> argsOut;

    auto args = ::GetCommandLineW();
    if (args == nullptr)
    {
        return argsOut;
    }

    int argc;
    auto argv = ::CommandLineToArgvW(args, &argc);
    if (!argv)
    {
        return argsOut;
    }

    for(int i = 0; i < argc; ++i)
    {
        argsOut.push_back(argv[i]);
    }

    ::LocalFree(argv);
    
    return argsOut;
}


std::wstring GetConfigPath()
{
    std::wstring configPath;

    auto args = ::GetArguments();
    if (args.size() > 1)
    {
        configPath = args[1];
    }
    else
    {
        auto processPath = ::GetFullModulePath();
        std::filesystem::path osPath(processPath);

        configPath = osPath.parent_path().append("nwn2mod.config");
    }

    return configPath;
}

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    PWSTR,
    int)
{
    auto configPath = ::GetConfigPath();

    auto result = NWN2Server::LoadServer(configPath);
    if (!result)
    {
        std::clog << result.error();
        return 1;
    }

    return 0;
}