#pragma once
#include <string>
#include <optional>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace nlohmann {
    /// <summary>Lets <c>nlohmann::json</c> (de)serialize <c>std::optional&lt;T&gt;</c>, as <c>null</c> when empty.</summary>
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        /// <summary>Serializes a <c>std::optional&lt;T&gt;</c> to JSON, as <c>null</c> when empty.</summary>
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

        /// <summary>Deserializes a <c>std::optional&lt;T&gt;</c> from JSON, as empty when the value is <c>null</c>.</summary>
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