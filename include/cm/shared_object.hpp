#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace cm::inline api {
    constexpr struct self_t {
        static const self_t instance;

        self_t(const self_t&)            = delete;
        self_t& operator=(const self_t&) = delete;

    private:
        self_t() = default;
    } self_t::instance;

    inline const self_t& self = self_t::instance;

    struct shared_object {
        explicit shared_object(void* handle);

        explicit shared_object(const self_t&);

        [[nodiscard]] auto get_native_handle() const -> void*;

        [[nodiscard]] auto get_base_address() const -> std::byte*;

        [[nodiscard]] auto get_file_path() const -> const std::filesystem::path&;

        [[nodiscard]] auto get_section_range(std::string_view section) const -> std::optional<std::span<std::byte>>;

        [[nodiscard]] auto get_persistent_data() const -> std::vector<std::byte>&;

        [[noreturn]] void exit_thread_and_reload() const;

    private:
        struct impl;

        std::unique_ptr<impl, void (*)(impl*)> p_impl;
    };
}
