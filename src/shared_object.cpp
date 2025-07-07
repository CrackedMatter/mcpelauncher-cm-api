#include "string_hash.hpp"
#include <cm/shared_object.hpp>
#include <cstddef>
#include <dlfcn.h>
#include <elf.h>
#include <expected>
#include <filesystem>
#include <fstream>
#include <link.h>
#include <string>
#include <utility>
#include <vector>

namespace cm {
    enum class parse_elf_file_error {
        failed_to_open_file,
        failed_to_read_elf_header,
        elf_header_mismatch,
        failed_to_read_section_headers,
        failed_to_read_section_header_string_table,
    };

    struct shared_object_impl {
        void* native_handle{};

        std::byte* base_address{};

        std::filesystem::path file_path{};

        unordered_string_map<std::span<std::byte>> section_ranges{};

        auto parse_elf_file() -> std::expected<void, parse_elf_file_error>;
    };

    struct shared_object::impl : shared_object_impl {};

    shared_object::shared_object(void* handle) : p_impl{new impl{handle}, [](impl* p) { delete p; }} {
        dl_iterate_phdr(
            [](dl_phdr_info* info, size_t, void* data) {
                auto p_impl = static_cast<impl*>(data);

                if (auto h = dlopen(info->dlpi_name, RTLD_NOLOAD); dlclose(h), h != p_impl->native_handle)
                    return 0;
                p_impl->base_address = reinterpret_cast<std::byte*>(info->dlpi_addr);
                p_impl->file_path    = info->dlpi_name;
                return 1;
            },
            p_impl.get());

        if (auto e = p_impl->parse_elf_file(); !e.has_value()) {
            // TODO
        }
    }

    shared_object::shared_object(const self_t& caller)
        : shared_object{[&] {
              Dl_info info;
              dladdr(&caller, &info);
              auto handle = dlopen(info.dli_fname, 0);
              dlclose(handle);
              return handle;
          }()} {}

    void* shared_object::get_native_handle() const {
        return p_impl->native_handle;
    }

    std::byte* shared_object::get_base_address() const {
        return p_impl->base_address;
    }

    const std::filesystem::path& shared_object::get_file_path() const {
        return p_impl->file_path;
    }

    std::optional<std::span<std::byte>> shared_object::get_section_range(std::string_view section) const {
        auto it = p_impl->section_ranges.find(section);
        if (it == p_impl->section_ranges.end())
            return std::nullopt;
        return it->second;
    }

    std::vector<std::byte>& shared_object::get_persistent_data() const {
        static unordered_string_map<std::vector<std::byte>> data{};
        return data[p_impl->file_path];
    }

    std::expected<void, parse_elf_file_error> shared_object_impl::parse_elf_file() {
        std::ifstream file{this->file_path, std::ios::binary};
        if (!file)
            return std::unexpected{parse_elf_file_error::failed_to_open_file};

        ElfW(Ehdr) ehdr{};
        file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));
        if (!file)
            return std::unexpected{parse_elf_file_error::failed_to_read_elf_header};

        if (std::memcmp(&ehdr, this->base_address, sizeof(ehdr)) != 0)
            return std::unexpected{parse_elf_file_error::elf_header_mismatch};

        std::vector<ElfW(Shdr)> section_headers(ehdr.e_shnum);
        file.seekg(static_cast<std::streamoff>(ehdr.e_shoff), std::ios::beg);
        file.read(reinterpret_cast<char*>(section_headers.data()), static_cast<std::streamsize>(ehdr.e_shnum * sizeof(ElfW(Shdr))));
        if (!file)
            return std::unexpected{parse_elf_file_error::failed_to_read_section_headers};

        const ElfW(Shdr)& shstrtab_hdr = section_headers[ehdr.e_shstrndx];
        std::vector<char> shstrtab(shstrtab_hdr.sh_size);
        file.seekg(static_cast<std::streamoff>(shstrtab_hdr.sh_offset), std::ios::beg);
        file.read(shstrtab.data(), static_cast<std::streamsize>(shstrtab_hdr.sh_size));
        if (!file)
            return std::unexpected{parse_elf_file_error::failed_to_read_section_header_string_table};

        for (const auto& section_header : section_headers) {
            std::string name  = &shstrtab[section_header.sh_name];
            std::byte*  begin = this->base_address + section_header.sh_offset;
            std::size_t size  = section_header.sh_size;
            this->section_ranges.insert({std::move(name), {begin, size}});
        }

        return {};
    }
}
