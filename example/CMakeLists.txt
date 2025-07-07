add_library(mcpelauncher_cm_api_example SHARED example.cpp)

target_link_libraries(mcpelauncher_cm_api_example PRIVATE mcpelauncher_cm_api_import)

if(CM_API_INSTALL_EXAMPLE)
    install(TARGETS mcpelauncher_cm_api_example LIBRARY DESTINATION mods)
endif()
