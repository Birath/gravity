#include "free_controller.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace gravity {

free_controller::free_controller()
	: up{0, 1, 0} {}

auto free_controller::handle_keyboard(SDL_KeyboardEvent const& event) -> void {
	switch (event.keysym.scancode) {
		case SDL_SCANCODE_A: velocity.x = event.state == SDL_PRESSED ? -1.f : 0.f; break;
		case SDL_SCANCODE_D: velocity.x = event.state == SDL_PRESSED ? 1.f : 0.f; break;
		case SDL_SCANCODE_W: velocity.z = event.state == SDL_PRESSED ? 1.f : 0.f; break;
		case SDL_SCANCODE_S: velocity.z = event.state == SDL_PRESSED ? -1.f : 0.f; break;
		case SDL_SCANCODE_SPACE: velocity.y = event.state == SDL_PRESSED ? 1.f : 0.f; break;
		case SDL_SCANCODE_LCTRL: velocity.y = event.state == SDL_PRESSED ? -1.f : 0.f; break;
		default: break;
	}
}

auto free_controller::handle_mouse(SDL_MouseMotionEvent const& event) -> void {
	auto const x_offset{event.xrel};
	auto const y_offset{-event.yrel};

	yaw += x_offset;
	pitch = std::clamp(pitch + y_offset, -89.f, 89.f);
}

auto free_controller::update(float elapsed_time, float delta_time) -> void {
	(void)elapsed_time;
	glm::vec3 camera_front{
		glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
		glm::sin(glm::radians(pitch)),
		glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
	};
	
	position += velocity.z * glm::normalize(camera_front) * speed * delta_time;
	position += glm::cross(camera_front, up) * velocity.x * speed * delta_time;
	position += up * velocity.y * speed * delta_time;
	view = glm::lookAt(position, position + camera_front, up);
}
} // namespace gravity
