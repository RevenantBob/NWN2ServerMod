#pragma once
#include <string>
#include <optional>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        // Serialize: std::optional -> JSON
        static void to_json(json& j, const std::optional<T>& opt)
        {
            if (!opt.has_value())
            {
                j = nullptr;
            }
            else
            {
                j = *opt;
            }
        }

        // Deserialize: JSON -> std::optional
        static void from_json(const json& j, std::optional<T>& opt)
        {
            if (j.is_null())
            {
                opt = std::nullopt;
            }
            else
            {
                opt = j.get<T>();
            }
        }
    };
}