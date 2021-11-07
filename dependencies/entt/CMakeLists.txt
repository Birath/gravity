message(STATUS "Fetching entt...")

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG dd6863f71da1b360ec09c25912617a3423f08149 # 3.8.1
)

FetchContent_MakeAvailable(entt)

add_library(dependency_entt INTERFACE)
# target_include_directories(dependency_entt SYSTEM INTERFACE "${entt_SOURCE_DIR}/single_include")
target_link_libraries(dependency_entt INTERFACE EnTT::EnTT)
