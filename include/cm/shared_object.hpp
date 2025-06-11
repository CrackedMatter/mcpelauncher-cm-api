#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace cm::inline api {
    struct shared_object {
        explicit shared_object(void* handle);

        [[nodiscard]] auto get_native_handle() const -> void*;

        [[nodiscard]] auto get_base_address() const -> std::byte*;

        [[nodiscard]] auto get_file_path() const -> const std::filesystem::path&;

        [[nodiscard]] auto get_section_range(std::string_view section) const -> std::optional<std::span<std::byte>>;

    private:
        struct impl;

        std::unique_ptr<impl, void (*)(impl*)> p_impl;
    };
}
