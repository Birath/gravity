#ifndef FREE_CONTROLLER_H
#define FREE_CONTROLLER_H

#include <SDL.h>
#include <glm/glm.hpp>

namespace gravity {

class free_controller {
    
    public:
    free_controller();

    auto handle_mouse(SDL_MouseMotionEvent const& event) -> void;
    auto handle_keyboard(SDL_KeyboardEvent const& event) -> void;
    auto update(float elapsed_time, float delta_time) -> void;

    glm::vec3 const up;
    glm::mat4 view;
    private:

    float yaw{};
    float pitch{};
    bool first_mouse{true};
    glm::vec2 last_mouse_pos{};

    glm::vec3 position{};
    glm::vec3 velocity{};
    float speed{1};
    
};
} // namespace gravity

#endif