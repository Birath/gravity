message(STATUS "Fetching FreeType...")

FetchContent_Declare(
    FreeType
    GIT_REPOSITORY https://github.com/freetype/freetype
    GIT_TAG e186230678ee8e4ea4ac4797ece8125761e3225a # 2.6
)

FetchContent_MakeAvailable(FreeType)

add_library(dependency_FreeType INTERFACE)
target_include_directories(dependency_FreeType SYSTEM INTERFACE "${freetype_SOURCE_DIR}/include")
target_link_libraries(dependency_FreeType INTERFACE freetype)
