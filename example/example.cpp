#include <chrono>
#include <cm/rtti.hpp>
#include <cm/shared_object.hpp>
#include <dlfcn.h>
#include <print>
#include <pthread.h>
#include <thread>

cm::shared_object selfObj{cm::self};

extern "C" [[gnu::visibility("default")]] void mod_preinit() {
    std::println("Example mod path: {}", selfObj.get_file_path().native());
}

extern "C" [[gnu::visibility("default")]] void mod_init() {
    cm::shared_object mcObj{dlopen("libminecraftpe.so", 0)};

    std::println("MinecraftGame vtable count: {}",
                 cm::find_all_vtables(mcObj, "13MinecraftGame")
                     .transform([](auto&& v) { return v.size(); })
                     .value_or(0));
}

[[gnu::constructor]] void constructor() {
    std::println("Example mod loaded");

    pthread_t thread;
    pthread_create(
        &thread,
        nullptr,
        [](void*) -> void* {
            {
                auto& data = selfObj.get_persistent_data();
                if (data.size() < 4)
                    data.resize(4);

                auto count = ++reinterpret_cast<std::uint32_t&>(*data.data());
                std::println("{}", count);

                auto reloadPath = selfObj.get_file_path().parent_path() / "cm_api_example_reload";

                while (true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{100});

                    if (!std::filesystem::exists(reloadPath))
                        continue;

                    if (!std::filesystem::is_empty(reloadPath))
                        std::abort();

                    if (std::filesystem::remove(reloadPath))
                        break;
                }
            }

            selfObj.exit_thread_and_reload();
        },
        nullptr);
}

[[gnu::destructor]] void destructor() {
    std::println("Example mod unloaded");
}
