#include <cm/rtti.hpp>
#include <cm/shared_object.hpp>
#include <dlfcn.h>
#include <print>

extern "C" [[gnu::visibility("default")]] void mod_preinit() {}

extern "C" [[gnu::visibility("default")]] void mod_init() {
    cm::shared_object mcObj{dlopen("libminecraftpe.so", 0)};

    std::println("MinecraftGame vtable count: {}",
                 cm::find_all_vtables(mcObj, "13MinecraftGame")
                     .transform([](auto&& v) { return v.size(); })
                     .value_or(0));
}
