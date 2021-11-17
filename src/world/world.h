#ifndef WORLD_H
#define WORLD_H

#include <entt/entt.hpp>
#include <SDL.h>

#include "renderer.h"
#include "model.h"
#include "free_controller.h"

namespace gravity {

class world {
	entt::registry registry;
	glm::mat4 view{};
	size_t sphere_resolution{5};
	int asteroid_amount{2};
public:
	world();
	~world() = default;

	auto tick(float delta_time) -> void;
	auto update(float elpsed_time, float delta_time) -> void;
	auto draw(renderer& renderer,  float elapsed_time, float delta_time) const -> void;
	auto handle_event(SDL_Event const& event) -> void;
	free_controller controller{};
};

} // namespace gravity

#endif
