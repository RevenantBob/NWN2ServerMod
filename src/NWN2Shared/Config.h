#pragma once
#include "JSON.h"
#include <expected>
#include <filesystem>

struct Config
{
    std::string server_exe;
    std::string server_args;
    std::optional<std::string> server_directory;
    std::optional<std::string> loader_dll;
    std::optional<std::string> loader_log;
    std::optional<std::string> server_log;
    std::optional<bool> debug_log;
    std::optional<bool> std_log;

    static std::expected<Config, std::string> FromString(std::string_view src);
    static std::expected<Config, std::string> FromFile(std::filesystem::path path);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    Config,
    server_exe,
    server_args,
    server_directory,
    loader_dll,
    loader_log,
    server_log,
    debug_log,
    std_log)

