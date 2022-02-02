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
	, asteroid_model{shape::create_sphere(3, 0.1f)}
	{
	registry.set<gravity_system::gravity_constant>(10.f);
	
	position_compute_handle = gravity_compute_shader.generate_buffer(100, 0, GL_DYNAMIC_COPY);
	velocity_compute_handle = gravity_compute_shader.generate_buffer(100, 1, GL_DYNAMIC_COPY);
	// position_compute_shader.bind_buffer(position_compute_handle, 0);
	// position_compute_shader.bind_buffer(velocity_compute_handle, 1);

	// auto const test_entity = registry.create();
	// registry.emplace_or_replace<transform_component>(test_entity, glm::vec3{0.0, 0.0, 0.0});
	// registry.emplace_or_replace<physics_component>(test_entity, glm::vec3{0.0, 0.0, 0.0});
	// auto const resolution{5};
	// auto sphere{shape::create_sphere(resolution, 0.5f)};
	// registry.emplace_or_replace<renderable>(test_entity, sphere);
	// registry.emplace_or_replace<sphere_component>(test_entity, resolution, 0.5f);
	// auto const face_entity = registry.create();
	// registry.emplace_or_replace<renderable>(face_entity, shape::create_plane());
}

auto world::tick(float delta_time) -> void {
	EASY_FUNCTION();

	

	// std::vector<glm::vec4> position_buffer{};
	// EASY_BLOCK("CREATE GRAVITY POSITION");
	// std::transform(position_view.begin(), position_view.end(), std::back_inserter(position_buffer), 
	// [&registry = registry](entt::entity entity) {
	// 	auto const pos = registry.get<transform_component>(entity).position;
	// 	auto const mass = registry.get<physics_component>(entity).mass;
	// 	return glm::vec4{pos.x, pos.y, pos.z, mass};
	// });
	// EASY_END_BLOCK;

	// std::vector<glm::vec4> velocity_buffer{position_buffer.size(), glm::vec4{}};
	// position_compute_handle = gravity_compute_shader.generate_buffer(position_buffer.size() * sizeof(compute_vec3<float>), 2, GL_DYNAMIC_READ);
	// velocity_compute_handle = gravity_compute_shader.generate_buffer(position_buffer.size() * sizeof(compute_vec3<float>), 3, GL_DYNAMIC_COPY);
	EASY_BLOCK("VELOCITY SHADER");
	gravity_compute_shader.use();
	// gravity_compute_shader.regenerate_buffer(position_buffer, position_compute_handle);
	// gravity_compute_shader.regenerate_buffer(velocity_buffer, velocity_compute_handle);
	// gravity_compute_shader.upload(position_buffer, position_compute_handle);
	gravity_compute_shader.upload_uniform("delta_time", delta_time);
	gravity_compute_shader.upload_uniform("gravity_constant", registry.ctx<const gravity_system::gravity_constant>().value);
	auto const buffer_size{registry.view<physics_component>().size()};
	auto const workgroup_size{static_cast<unsigned int>(buffer_size / 32 + (buffer_size % 32 == 0 ? 0 : 1))};
	gravity_compute_shader.dispatch(std::max(workgroup_size, 1u), 1, 1);

	// gravity_compute_shader.read(velocity_buffer, velocity_compute_handle);
	EASY_END_BLOCK;
	EASY_BLOCK("POSITION SHADER");
	position_compute_shader.use();
	position_compute_shader.upload_uniform("delta_time", delta_time);
	position_compute_shader.dispatch(std::max(workgroup_size, 1u), 1, 1);
	std::vector<glm::vec4> position_buffer{};
	position_buffer.resize(buffer_size);
	position_compute_shader.read(position_buffer, position_compute_handle);
	EASY_END_BLOCK;
	// gravity_compute_shader.read(velocity_buffer, velocity_compute_handle);
	auto position_view = registry.view<transform_component, physics_component>();
	int i{0};
	EASY_BLOCK("PATCH");
	for (auto [entity, transform, physics] : position_view.each()) {
		// auto new_velocity{velocity_buffer[i++]};
		auto new_position{position_buffer[i++]};
		// physics.velocity += glm::vec3{new_velocity.x, new_velocity.y, new_velocity.z};
		// registry.patch<physics_component>(entity, [new_velocity, delta_time](auto& p) {
		// 	p.velocity += glm::vec3{new_velocity.x, new_velocity.y, new_velocity.z};
		// });
		auto velocity = physics.velocity;
		registry.patch<transform_component>(entity, [new_position, delta_time](auto& trans) {
			trans.position = new_position;
		});
	}

	// gravity_system::update(registry, delta_time);

	// auto sphere_view = registry.view<const transform_component, sphere_component, renderable>(entt::exclude<deletion_component>);

	// std::vector<entt::entity> marked_for_deletion{};

	// for (auto [entity, transform, sphere, renderable] : sphere_view.each()) {
	// 	for (auto [entity_other, transform_other, sphere_other, renderable_other] : sphere_view.each()) {
	// 		if (entity != entity_other) {
	// 			auto const distance{glm::distance2(transform.position, transform_other.position)};
	// 			if (distance < (sphere.radius + sphere_other.radius)) {
	// 				sphere.radius = std::cbrtf(std::powf(sphere.radius, 3.f) + std::powf(sphere_other.radius, 3.f));
	// 				sphere.resolution = std::max(sphere.resolution, sphere_other.resolution);
	// 				auto const name1{registry.get<name_component>(entity).name};
	// 				auto const name2{registry.get<name_component>(entity_other).name};
	// 				// fmt::print("NAME1 {}, NAME2 {}: SIZE: {}\n", name1, name2, renderable.model.meshes.size());
	// 				shape::regenerate_sphere(renderable.model, sphere.resolution, sphere.radius);
	// 				auto& physics = registry.get<physics_component>(entity);
	// 				auto& physics_other = registry.get<physics_component>(entity_other);
	// 				physics.mass += physics_other.mass;
	// 				// physics.velocity += physics_other.velocity;
	// 				// registry.emplace<physics_component>(entity, glm::vec3{0.f}, sphere.radius * 2);
	// 				registry.emplace_or_replace<deletion_component>(entity_other);
	// 				// fmt::print("DESTROYED ENTITY: {}\n", name2);
	// 			}
	// 		}
	// 	}
	// }
	// auto destroy_view = registry.view<deletion_component>();
	// registry.destroy(destroy_view.begin(), destroy_view.end());
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
	// ImGui::SliderFloat2("Band", asteroid_radii, 0.f, 100.f);
	// ImGui::DragFloatRange2("Band", &asteroid_inner_radius, &asteroid_outer_radius)
	if (ImGui::Button("Spawn Asteroids")) {
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
			auto asteroid_sphere{shape::create_sphere(3, 0.1f)};
			registry.emplace<renderable>(asteroid, asteroid_sphere);

			registry.emplace_or_replace<sphere_component>(asteroid, 3, 0.1f);
			registry.emplace<name_component>(asteroid, "ASTEROID ");
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
	auto view = registry.view<const transform_component>();
	// for (auto&& [entity, renderable, position] : view.each()) {
	// 	renderer.draw_model(renderable.model, {position.position}, elapsed_time, delta_time);
	// }
	std::vector<glm::mat4> matrices{};
	EASY_BLOCK("CREATE MATRICES");
	view.each([&](auto entity, auto const& transform) {
		(void)entity;
		matrices.emplace_back(glm::translate(glm::mat4{1.f}, transform.position));
		// matrices.emplace_back(glm::rotate(glm::translate(glm::mat4{1.f}, transform.position), glm::radians(elapsed_time * 100.f), glm::vec3{0.f, 1.f, 0.f}));
	});
	EASY_END_BLOCK;
	// std::transform(std::cbegin(view), std::cend(view), std::back_inserter(matrices), [](auto entity) {
	// 	auto transform = 
	// 	return glm::translate(glm::mat4{1.f}, transform.position);
	// });
	std::span<glm::mat4> matrices_span{matrices};
	renderer.draw_model_instanced(asteroid_model, matrices_span);
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
