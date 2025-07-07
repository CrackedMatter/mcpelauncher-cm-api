#include <cm/rtti.hpp>
#include <libhat/scanner.hpp>
#include <libhat/signature.hpp>
#include <ranges>

namespace cm {
    std::expected<void*, find_vtable_error> api::find_typeinfo(std::span<std::byte> rodata, std::span<std::byte> data_rel_ro, std::string_view name) {
        hat::signature name_signature;
        name_signature.reserve(name.size() + 1);
        name_signature.assign_range(std::as_bytes(std::span{name}));
        name_signature.emplace_back(std::byte{'\0'});

        auto typeinfo_name = hat::find_pattern(rodata, name_signature);
        if (!typeinfo_name.has_result())
            return std::unexpected{find_vtable_error::typeinfo_name};

        auto typeinfo = hat::find_pattern(data_rel_ro, hat::object_to_signature(typeinfo_name.get()));
        if (!typeinfo.has_result())
            return std::unexpected{find_vtable_error::typeinfo};

        return typeinfo.get() - sizeof(void*);
    }

    std::expected<void*, find_vtable_error> api::find_typeinfo(const shared_object& object, std::string_view name) {
        auto rodata      = object.get_section_range(".rodata");
        auto data_rel_ro = object.get_section_range(".data.rel.ro");
        if (!rodata.has_value() || !data_rel_ro.has_value())
            return std::unexpected{find_vtable_error::section};

        return find_typeinfo(*rodata, *data_rel_ro, name);
    }

    std::expected<void*, find_vtable_error> api::find_vtable(std::span<std::byte> rodata, std::span<std::byte> data_rel_ro, std::string_view name) {
        auto typeinfo = find_typeinfo(rodata, data_rel_ro, name);
        if (!typeinfo.has_value())
            return std::unexpected{typeinfo.error()};

        auto vtable = hat::find_pattern(data_rel_ro, hat::object_to_signature(*typeinfo));
        if (!vtable.has_result())
            return std::unexpected{find_vtable_error::vtable};

        return vtable.get() + sizeof(void*);
    }

    std::expected<void*, find_vtable_error> api::find_vtable(const shared_object& object, std::string_view name) {
        auto rodata      = object.get_section_range(".rodata");
        auto data_rel_ro = object.get_section_range(".data.rel.ro");
        if (!rodata.has_value() || !data_rel_ro.has_value())
            return std::unexpected{find_vtable_error::section};

        return find_vtable(*rodata, *data_rel_ro, name);
    }

    std::expected<std::vector<void*>, find_vtable_error> api::find_all_vtables(std::span<std::byte> rodata, std::span<std::byte> data_rel_ro, std::string_view name) {
        struct vtables_insert_iterator {
            using difference_type = std::ptrdiff_t;

            std::vector<void*>* container;

            constexpr vtables_insert_iterator& operator=(hat::scan_result value) {
                container->push_back(value.get() + sizeof(void*));
                return *this;
            }

            constexpr vtables_insert_iterator& operator*() { return *this; }
            constexpr vtables_insert_iterator& operator++() { return *this; }
            constexpr vtables_insert_iterator& operator++(int) { return *this; }
        };

        std::expected<std::vector<void*>, find_vtable_error> result;

        auto typeinfo = find_typeinfo(rodata, data_rel_ro, name);
        if (!typeinfo.has_value()) {
            result = std::unexpected{typeinfo.error()};
            return result;
        }

        if (hat::find_all_pattern(data_rel_ro, vtables_insert_iterator{&*result}, hat::object_to_signature(*typeinfo)) == 0)
            result = std::unexpected{find_vtable_error::vtable};

        return result;
    }

    std::expected<std::vector<void*>, find_vtable_error> api::find_all_vtables(const shared_object& object, std::string_view name) {
        auto rodata      = object.get_section_range(".rodata");
        auto data_rel_ro = object.get_section_range(".data.rel.ro");
        if (!rodata.has_value() || !data_rel_ro.has_value())
            return std::unexpected{find_vtable_error::section};

        return find_all_vtables(*rodata, *data_rel_ro, name);
    }
}
