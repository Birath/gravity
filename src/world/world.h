#ifndef WORLD_H
#define WORLD_H

#include <entt/entt.hpp>
#include <SDL.h>
#include <random>

#include "renderer.h"
#include "model.h"
#include "compute.h"
#include "free_controller.h"

namespace gravity {

class world {
	compute gravity_compute_shader;
	compute position_compute_shader;
	unsigned int position_compute_handle;
	unsigned int velocity_compute_handle;

	std::random_device r;
	std::default_random_engine random_engine;

	entt::registry registry;
	glm::mat4 view{};
	
	int sphere_resolution{5};
	int asteroid_amount{2};

	float asteroid_inner_radius{15.f};
	float asteroid_outer_radius{25.f};	

public:
	world();
	~world() = default;

	auto tick(float delta_time) -> void;
	auto update(float elapsed_time, float delta_time) -> void;
	auto draw(renderer& renderer,  float elapsed_time, float delta_time) const -> void;
	auto handle_event(SDL_Event const& event) -> void;
	free_controller controller{};
};

} // namespace gravity

#endif
