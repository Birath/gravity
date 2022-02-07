#include "world.h"

#include "components.h"
#include "gravity_system.h"
#include "shape.h"

#include "randomness.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fmt/core.h>
#include <glm/gtx/norm.hpp>
#include <imgui.h>
#include <random>
#include <easy/profiler.h>

namespace gravity {

world::world()
	: gravity_compute_shader{compute{std::filesystem::path{"assets/shaders/gravity.glsl"}}}
	, position_compute_shader{compute{std::filesystem::path{"assets/shaders/positions.glsl"}}}
	, position_compute_handle{0}
	, velocity_compute_handle{0}
	, random_engine{r()}
	{
	registry.set<gravity_system::gravity_constant>(10.f);
	
	position_compute_handle = gravity_compute_shader.generate_buffer(100, 0, GL_DYNAMIC_COPY);
	velocity_compute_handle = gravity_compute_shader.generate_buffer(100, 1, GL_DYNAMIC_COPY);
}

auto world::tick(float delta_time) -> void {
	EASY_FUNCTION();

	EASY_BLOCK("VELOCITY SHADER");
	gravity_compute_shader.use();
	gravity_compute_shader.upload_uniform("delta_time", delta_time);
	gravity_compute_shader.upload_uniform("gravity_constant", registry.ctx<const gravity_system::gravity_constant>().value);
	auto const buffer_size{registry.view<physics_component>().size()};
	auto const workgroup_size{static_cast<unsigned int>(buffer_size / 32 + (buffer_size % 32 == 0 ? 0 : 1))};
	gravity_compute_shader.dispatch(std::max(workgroup_size, 1u), 1, 1);

	EASY_END_BLOCK;

	EASY_BLOCK("POSITION SHADER");
	position_compute_shader.use();
	position_compute_shader.upload_uniform("delta_time", delta_time);
	position_compute_shader.dispatch(std::max(workgroup_size, 1u), 1, 1);
	EASY_END_BLOCK;
}

auto world::update(float elapsed_time, float delta_time) -> void {
	EASY_FUNCTION();
	controller.update(elapsed_time, delta_time);
	auto spheres = registry.view<sphere_component, renderable>();

	ImGui::Begin("Settings");
	constexpr float min_gravity{0.1f};
	constexpr float max_gravity{10.f};
	auto& g_c = registry.ctx<gravity_system::gravity_constant>();
	ImGui::SliderFloat("Gravity Constant", &g_c.value, min_gravity, max_gravity, "%.3f", 1.f);
	if (ImGui::SliderScalar("Sphere resolution", ImGuiDataType_U8, &sphere_resolution, &shape::min_sphere_resolution, &shape::max_sphere_resolution)) {
		for (auto&& [entity, sphere, renderable] : spheres.each()) {
			auto const resolution{sphere_resolution};
			if (sphere.resolution != resolution) {
				shape::regenerate_sphere(renderable.model, resolution, sphere.radius);
				sphere.resolution = resolution;
			}
		}
	}

	ImGui::End();
	ImGui::Begin("Spawn");
	if (ImGui::Button("Spawn sphere")) {
		auto const sphere_entity = registry.create();
		registry.emplace_or_replace<physics_component>(sphere_entity, glm::vec3{0.0, 0.0, 0.0}, 1.f);
		registry.emplace_or_replace<transform_component>(sphere_entity, controller.view_position(2.f));
		auto const resolution{5};
		auto sphere{shape::create_sphere(resolution, 0.5f)};
		registry.emplace_or_replace<renderable>(sphere_entity, sphere);
		registry.emplace<name_component>(sphere_entity, "SPHERE");
		registry.emplace_or_replace<sphere_component>(sphere_entity, resolution, 0.5f);
	}

	if (ImGui::Button("Spawn moon system")) {
		position_compute_shader.use();
		position_compute_shader.clear_buffer(position_compute_handle);
		gravity_compute_shader.use();
		gravity_compute_shader.clear_buffer(velocity_compute_handle);
		registry.clear();

		auto const planet = registry.create();
		auto const moon = registry.create();
		registry.emplace<name_component>(planet, "PLANET");
		registry.emplace<name_component>(moon, "MOON");

		auto const& planet_physics = registry.emplace<physics_component>(planet, glm::vec3{0.f}, 1000.f);
		auto& moon_physics = registry.emplace<physics_component>(moon, glm::vec3{0.f}, 1.f);

		registry.emplace<transform_component>(planet, glm::vec3{0.f});
		auto& moon_transform = registry.emplace<transform_component>(moon, glm::vec3{10.f, 0.f, 0.f});

		auto const init_velocity{
			std::sqrt(registry.ctx<const gravity_system::gravity_constant>().value * (planet_physics.mass + 1 / moon_physics.mass) / moon_transform.position.x)};
		moon_physics.velocity.z = init_velocity;

		auto sphere{shape::create_sphere(15, 0.5f)};
		registry.emplace_or_replace<renderable>(moon, sphere);
		registry.emplace_or_replace<sphere_component>(moon, 15, 0.5f);
		auto planet_sphere{shape::create_sphere(15, 5.f)};
		registry.emplace_or_replace<renderable>(planet, planet_sphere);
		registry.emplace_or_replace<sphere_component>(planet, 15, 5.f);
	}
	ImGui::DragFloatRange2("Band", &asteroid_inner_radius, &asteroid_outer_radius);

	if (ImGui::Button("Spawn Asteroids")) {
		gravity_compute_shader.use();
		gravity_compute_shader.clear_buffer(position_compute_handle);
		gravity_compute_shader.use();
		gravity_compute_shader.clear_buffer(velocity_compute_handle);
		registry.clear();

		auto const planet = registry.create();
		auto& planet_physics = registry.emplace<physics_component>(planet, glm::vec3{0.f}, 1000.f);
		registry.emplace<transform_component>(planet, glm::vec3{0.f});
		auto planet_sphere{shape::create_sphere(15, 5.f)};
		registry.emplace_or_replace<renderable>(planet, planet_sphere);
		registry.emplace_or_replace<sphere_component>(planet, 15, 5.f);
		registry.emplace<name_component>(planet, "PLANET");
		
		std::uniform_real_distribution<float> pos_dist(asteroid_inner_radius, asteroid_outer_radius);
		for (size_t i{0}; i < asteroid_amount; ++i) {
			auto const asteroid = registry.create();
			auto& asteroid_physics = registry.emplace<physics_component>(asteroid, glm::vec3{0.f}, 0.01f);

			auto const init_pos{random::generate_point_in_sphere(pos_dist, random_engine)};
			
			// auto& asteroid_transform = registry.emplace<transform_component>(asteroid, glm::vec3{pos_dist(el), 0.f, pos_dist(el)});
			auto& asteroid_transform = registry.emplace<transform_component>(asteroid, init_pos);
			auto const init_velocity_direction{glm::normalize(glm::cross(-asteroid_transform.position, glm::vec3{0.f, 1.f, 0.f}))};

			auto const init_velocity{std::sqrt(
				registry.ctx<const gravity_system::gravity_constant>().value * (planet_physics.mass + 1 / asteroid_physics.mass) / glm::length(asteroid_transform.position))};
			asteroid_physics.velocity = init_velocity_direction * init_velocity;

			registry.emplace<name_component>(asteroid, "ASTEROID");
			registry.emplace<instanced_component>(asteroid);
		}
		std::vector<glm::vec4> velocity_buffer{};
		auto physics = registry.view<const physics_component>();
		std::transform(physics.begin(), physics.end(), std::back_inserter(velocity_buffer), 
		[&registry = registry](entt::entity entity) {
			auto const vel = registry.get<physics_component>(entity).velocity;
			return glm::vec4{vel.x, vel.y, vel.z, 0.f};
		});

		auto position_view = registry.view<transform_component, physics_component>();
		std::vector<glm::vec4> position_buffer{};
		std::transform(position_view.begin(), position_view.end(), std::back_inserter(position_buffer), 
		[&registry = registry](entt::entity entity) {
			auto const pos = registry.get<transform_component>(entity).position;
			auto const mass = registry.get<physics_component>(entity).mass;
			return glm::vec4{pos.x, pos.y, pos.z, mass};
		});

		gravity_compute_shader.regenerate_buffer(velocity_buffer, velocity_compute_handle);
		gravity_compute_shader.upload(velocity_buffer, velocity_compute_handle);

		gravity_compute_shader.regenerate_buffer(position_buffer, position_compute_handle);
		gravity_compute_shader.upload(position_buffer, position_compute_handle);

	}

	ImGui::InputInt("Asteroid amount", &asteroid_amount);

	ImGui::End();
};

auto world::draw(renderer& renderer, float elapsed_time, float delta_time) const -> void {
	EASY_FUNCTION();
	(void)delta_time;
	// https://learnopengl.com/Advanced-OpenGL/Instancing
	auto instanced_view = registry.view<const instanced_component>();
	renderer.draw_asteroid_instanced(instanced_view.size());

	auto view = registry.view<const transform_component, const renderable>(entt::exclude<instanced_component>);
	renderer.start_non_instanced();
	for (auto&& [entity, transform, renderable] : view.each()) {
		renderer.draw_model(renderable.model, transform.position, elapsed_time, delta_time);
	}
}

auto world::handle_event(SDL_Event const& e) -> void {
	switch (e.type) {
		case SDL_MOUSEMOTION: controller.handle_mouse(e.motion); break;
		case SDL_KEYDOWN: controller.handle_keyboard(e.key); break;
		case SDL_KEYUP: controller.handle_keyboard(e.key); break;

		default: break;
	}
}

} // namespace gravity
