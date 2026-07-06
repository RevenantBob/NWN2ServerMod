#include "Config.h"
#include <fstream>

std::expected<Config, std::string> Config::FromString(std::string_view src)
{
    try
    {
        auto c = json::parse(src).get<Config>();

        return c;
    }
    catch (nlohmann::json::exception& e)
    {
        return std::unexpected(e.what());
    }
    catch (std::exception& e)
    {
        return std::unexpected(e.what());
    }
    catch (...)
    {
        return std::unexpected("unexpected exception");
    }
}

std::expected<Config, std::string> Config::FromFile(std::filesystem::path path)
{
    try
    {
        std::ifstream file(path);
        auto c = json::parse(file).get<Config>();

        return c;
    }
    catch (nlohmann::json::exception& e)
    {
        return std::unexpected(e.what());
    }
    catch (std::exception& e)
    {
        return std::unexpected(e.what());
    }
    catch (...)
    {
        return std::unexpected("unexpected exception");
    }
}
