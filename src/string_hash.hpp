#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace cm {
    struct string_hash {
        using hash_type      = std::hash<std::string_view>;
        using is_transparent = void;

        [[nodiscard]] std::size_t operator()(const char* str) const { return hash_type{}(str); }
        [[nodiscard]] std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
        [[nodiscard]] std::size_t operator()(const std::string& str) const { return hash_type{}(str); }
    };

    template<typename T>
    using unordered_string_map = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;
}
