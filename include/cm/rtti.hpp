#pragma once

#include "shared_object.hpp"
#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace cm::inline api {
    enum class find_vtable_error {
        section,
        typeinfo_name,
        typeinfo,
        vtable,
    };

    [[nodiscard]] auto find_typeinfo(
        std::span<std::byte> rodata,
        std::span<std::byte> data_rel_ro,
        std::string_view     name)
        -> std::expected<void*, find_vtable_error>;

    [[nodiscard]] auto find_typeinfo(
        const shared_object& object,
        std::string_view     name)
        -> std::expected<void*, find_vtable_error>;

    [[nodiscard]] auto find_vtable(
        std::span<std::byte> rodata,
        std::span<std::byte> data_rel_ro,
        std::string_view     name)
        -> std::expected<void*, find_vtable_error>;

    [[nodiscard]] auto find_vtable(
        const shared_object& object,
        std::string_view     name)
        -> std::expected<void*, find_vtable_error>;

    [[nodiscard]] auto find_all_vtables(
        std::span<std::byte> rodata,
        std::span<std::byte> data_rel_ro,
        std::string_view     name)
        -> std::expected<std::vector<void*>, find_vtable_error>;

    [[nodiscard]] auto find_all_vtables(
        const shared_object& object,
        std::string_view     name)
        -> std::expected<std::vector<void*>, find_vtable_error>;
}
