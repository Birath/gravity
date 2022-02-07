include(FetchContent)

add_subdirectory(fmt)
add_subdirectory(entt)
add_subdirectory(OpenGL)
add_subdirectory(GLEW) # Depends on OpenGL.
add_subdirectory(SDL2)
add_subdirectory(glm)
add_subdirectory(stb_image)
add_subdirectory(tiny_obj_loader)
add_subdirectory(imgui)
