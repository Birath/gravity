cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project("Gravity" LANGUAGES CXX C)

option(BUILD_SHARED_LIBS "Build dependencies as shared libraries" OFF)
option(USE_CLANG_TIDY "Use clang-tidy for static analysis warnings" OFF)
option(USE_INCLUDE_WHAT_YOU_USE "Use include-what-you-use for include warnings" OFF)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(gravity ${SOURCE_FILES})
target_include_directories(gravity PRIVATE "include")
target_include_directories(gravity PRIVATE "src" "src/world" "src/resources")
target_compile_features(gravity PRIVATE cxx_std_20 c_std_11)
target_compile_options(gravity PRIVATE
	$<$<CXX_COMPILER_ID:GNU>:     -Wall -Wextra   -Wpedantic      -Werror                     $<$<CONFIG:Debug>:-g>   $<$<NOT:$<CONFIG:Debug>>:-O3>>
	$<$<CXX_COMPILER_ID:Clang>:   -Wall -Wextra   -Wpedantic      -Werror                     $<$<CONFIG:Debug>:-g>   $<$<NOT:$<CONFIG:Debug>>:-O3>>
	$<$<CXX_COMPILER_ID:MSVC>:    /W3             /permissive-    /WX     /wd4996     /utf-8  $<$<CONFIG:Debug>:/Od>  $<$<NOT:$<CONFIG:Debug>>:/Ot>>)

if(USE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy REQUIRED)
    set_property(TARGET gravity PROPERTY C_CLANG_TIDY ${CLANG_TIDY} "-header-filter=src/")
endif()

if(USE_INCLUDE_WHAT_YOU_USE)
    find_program(INCLUDE_WHAT_YOU_USE NAMES include-what-you-use REQUIRED)
    set_property(TARGET gravity PROPERTY C_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE})
endif()

add_subdirectory(dependencies)
add_subdirectory(utils)

target_link_libraries(gravity PRIVATE
	dependency_entt
	dependency_fmt
	dependency_FreeType
	dependency_OpenGL
	dependency_GLEW
	dependency_SDL2
	dependency_glm
	dependency_imgui
	stb_image
	utils
	$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:m>)

if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET gravity
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:GLEW>
            $<TARGET_FILE:SDL2>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

include(GNUInstallDirs)

set_target_properties(gravity PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_INSTALL_BINDIR}")

install(TARGETS gravity
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/assets/" DESTINATION "${CMAKE_INSTALL_BINDIR}/assets/")
