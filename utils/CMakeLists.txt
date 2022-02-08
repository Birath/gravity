add_library(utils INTERFACE)
target_include_directories(utils SYSTEM INTERFACE .)
add_subdirectory(stubs)

if (NOT USE_PROFILER)
    target_link_libraries(utils INTERFACE easy_stub)
endif()
