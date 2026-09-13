#pragma once
#include "JSON.h"
#include <expected>
#include <filesystem>
#include <vector>

/// <summary>The <c>nwn2mod.config</c> JSON schema; see the README for the full field reference.</summary>
struct Config
{
    /// <summary>The full path to <c>NWN2Server64.exe</c>.</summary>
    std::string server_exe;

    /// <summary>The arguments passed to <see cref="server_exe"/> when executed.</summary>
    std::string server_args;

    /// <summary>The working directory for <see cref="server_exe"/>. Defaults to its parent path.</summary>
    std::optional<std::string> server_directory;

    /// <summary>The full path to <c>NWN2ModLoader.dll</c>. Defaults to the same parent path as the launcher executable.</summary>
    std::optional<std::string> loader_dll;

    /// <summary>The full path to a log file written to by <c>NWN2ModLoader.dll</c>. No log file is created if omitted.</summary>
    std::optional<std::string> loader_log;

    /// <summary>The full path to a log file written to by the launcher. No log file is created if omitted.</summary>
    std::optional<std::string> server_log;

    /// <summary>If <see langword="true"/>, logs are additionally emitted via <c>OutputDebugString</c>. Defaults to <see langword="false"/>.</summary>
    std::optional<bool> debug_log;

    /// <summary>If <see langword="true"/>, logs are additionally emitted to stdout/stderr. Defaults to <see langword="true"/>.</summary>
    std::optional<bool> std_log;

    /// <summary>Full paths to plugin DLLs to load, in order. No plugins are loaded if omitted.</summary>
    std::optional<std::vector<std::string>> plugins;

    /// <summary>Parses a config from a JSON string.</summary>
    /// <param name="src">The JSON text.</param>
    /// <returns>The parsed config, or an error message on failure.</returns>
    static std::expected<Config, std::string> FromString(std::string_view src);

    /// <summary>Parses a config from a JSON file.</summary>
    /// <param name="path">The full path to the config file.</param>
    /// <returns>The parsed config, or an error message on failure.</returns>
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
    std_log,
    plugins)

