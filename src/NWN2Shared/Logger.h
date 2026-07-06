#pragma once
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <string>
#include <format>
#include <source_location>
#include <chrono>
#include <mutex>

#include "Data.h"

class Logger
{
public:
    enum class Level {
        Info,
        Warning,
        Error
    };

    // Constructor accepting requirements
    Logger(
        std::optional<std::filesystem::path> filePath = std::nullopt,
        bool stdOut = false,
        bool debugOutput = false)
        : _DebugOutput(debugOutput)
        , _StdOutput(stdOut)
    {
        if (filePath)
        {
            // C++23: open with ios::app to append logs safely
            _File.open(*filePath, std::ios::out | std::ios::app);
        }
    }

    ~Logger()
    {
        if (_File.is_open())
        {
            _File.close();
        }
    }

    // Disable copying due to the file stream and mutex
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template <typename... Args>
    void operator()(Level level, std::format_string<Args...> fmt, Args&&... args)
    {
        // Forward the arguments to your core log function
        log(level, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(std::format_string<Args...> fmt, Args&&... args)
    {
        // Forward the arguments to your core log function
        log(Level::Info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args)
    {
        // Forward the arguments to your core log function
        log(Level::Info, fmt, std::forward<Args>(args)...);
    }
    // Core log method utilizing C++20/23 compile-time formatting & source location
    template <typename... Args>
    void log(Level level,
        std::format_string<Args...> fmt,
        Args&&... args)
    {
        // Format the user's message
        std::string msg = std::format(fmt, std::forward<Args>(args)...);

        auto now = std::chrono::system_clock::now();

        // Construct the full log entry line
        // C++23 allows direct formatting of chrono timestamps via std::format
        std::string logLine = std::format("[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n",
            now,
            LevelToString(level),
            msg);

        // Thread-safe writing to targets
        std::lock_guard<std::mutex> lock(_Lock);

        // Standard Output / Error
        if (_StdOutput)
        {
            if (level == Level::Error)
            {
                std::clog << logLine;
            }
            else
            {
                std::cout << logLine;
            }
        }

        if (_File.is_open())
        {
            _File << logLine;
            _File.flush(); // Ensure visibility on crash
        }

        if (_DebugOutput)
        {
            // OutputDebugStringW requires wide characters
            auto debugLine = ToWString(logLine);
            OutputDebugStringW(debugLine.c_str());
        }
    }

    static std::string Source(const std::source_location loc = std::source_location::current())
    {
        // Extract basic filename from path
        std::filesystem::path file_path(loc.file_name());
        std::string filename = file_path.filename().string();

        return std::format("{}:{}", filename, loc.line());
    }

private:
    bool _DebugOutput;
    bool _StdOutput;
    std::ofstream _File;
    std::mutex _Lock;

    static constexpr const char* LevelToString(Level level)
    {
        switch (level)
        {
        case Level::Info: return "I";
        case Level::Warning: return "W";
        case Level::Error: return "E";
        }

        return "UNKNOWN";
    }
};
