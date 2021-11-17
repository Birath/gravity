#include "world.h"

#include "shape.h"
#include "gravity_system.h"
#include "components.h"

#include <algorithm>
#include <fmt/core.h>
#include <glm/gtx/norm.hpp>
#include <imgui.h>
#include <cmath>
#include <random>

namespace gravity {

world::world()
	: registry{} {
		registry.set<gravity_system::gravity_constant>(10.f);
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
	
	gravity_system::update(registry, delta_time);

	auto sphere_view = registry.view<const transform_component, sphere_component, renderable>(entt::exclude<deletion_component>);

	std::vector<entt::entity> marked_for_deletion{};
	
	for (auto [entity, transform, sphere, renderable] : sphere_view.each()) {

		for (auto [entity_other, transform_other, sphere_other, renderable_other] : sphere_view.each()) {
			if (entity != entity_other) {
				auto const distance{glm::distance2(transform.position, transform_other.position)};
				if (distance < (sphere.radius + sphere_other.radius)) {
					sphere.radius = std::cbrtf(std::powf(sphere.radius, 3.f) + std::powf(sphere_other.radius, 3.f));
					sphere.resolution =  std::max(sphere.resolution, sphere_other.resolution);
					auto const name1{registry.get<name_component>(entity).name};
					auto const name2{registry.get<name_component>(entity_other).name};
					// fmt::print("NAME1 {}, NAME2 {}: SIZE: {}\n", name1, name2, renderable.model.meshes.size());
					shape::regenerate_sphere(renderable.model, sphere.resolution, sphere.radius);
					auto& physics = registry.get<physics_component>(entity);
					auto& physics_other = registry.get<physics_component>(entity_other);
					physics.mass += physics_other.mass;
					// physics.velocity += physics_other.velocity;
					// registry.emplace<physics_component>(entity, glm::vec3{0.f}, sphere.radius * 2);
					registry.emplace_or_replace<deletion_component>(entity_other);
					// fmt::print("DESTROYED ENTITY: {}\n", name2);
				}
			}
		}
	}
	auto destroy_view = registry.view<deletion_component>();
	registry.destroy(destroy_view.begin(), destroy_view.end());
	
}

auto world::update(float elapsed_time, float delta_time) -> void {
	controller.update(elapsed_time, delta_time);
	auto spheres = registry.view<sphere_component, renderable>();

	ImGui::Begin("Settings");
	constexpr float min_gravity{0.1f};
	constexpr float max_gravity{10.f};
	auto& g_c = registry.ctx<gravity_system::gravity_constant>();
	ImGui::SliderScalar("Sphere resolution", ImGuiDataType_U8, &sphere_resolution, &shape::min_sphere_resolution, &shape::max_sphere_resolution);
	ImGui::SliderFloat("Gravity Constant", &g_c.value, min_gravity, max_gravity, "%.3f", 1.f);
	for (auto&& [entity, sphere, renderable] : spheres.each()) {
		auto const resolution{sphere_resolution};
		if (sphere.resolution != resolution) {
			shape::regenerate_sphere(renderable.model, sphere.resolution, sphere.radius);
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

		auto const init_velocity{std::sqrt(
			registry.ctx<const gravity_system::gravity_constant>().value *
			(planet_physics.mass + 1 / moon_physics.mass) / moon_transform.position.x
		)};
		moon_physics.velocity.z = init_velocity;

		auto sphere{shape::create_sphere(15, 0.5f)};
		registry.emplace_or_replace<renderable>(moon, sphere);
		registry.emplace_or_replace<sphere_component>(moon, 15, 0.5f);
		auto planet_sphere{shape::create_sphere(15, 5.f)};
		registry.emplace_or_replace<renderable>(planet, planet_sphere);
		registry.emplace_or_replace<sphere_component>(planet, 15, 5.f);

	}

	if (ImGui::Button("Spawn Asteroids")) {
		registry.clear();

		auto const planet = registry.create();
		auto& planet_physics = registry.emplace<physics_component>(planet, glm::vec3{0.f}, 1000.f);
		registry.emplace<transform_component>(planet, glm::vec3{0.f});
		auto planet_sphere{shape::create_sphere(15, 5.f)};
		registry.emplace_or_replace<renderable>(planet, planet_sphere);
		registry.emplace_or_replace<sphere_component>(planet, 15, 5.f);
		registry.emplace<name_component>(planet, "PLANET");
		std::random_device r;
		std::default_random_engine el{r()};
		std::uniform_real_distribution<float> pos_dist(15.f, 25.f);
		for(size_t i {0}; i < asteroid_amount ; ++i) {
			auto const asteroid = registry.create();
			auto& asteroid_physics = registry.emplace<physics_component>(asteroid, glm::vec3{0.f}, 0.01f);
			auto& asteroid_transform = registry.emplace<transform_component>(asteroid, glm::vec3{pos_dist(el), 0.f, pos_dist(el)});

			auto const init_velocity_direction{glm::normalize(glm::cross(-asteroid_transform.position, glm::vec3{0.f, 1.f, 0.f}))};

			auto const init_velocity{std::sqrt(
				registry.ctx<const gravity_system::gravity_constant>().value *
				(planet_physics.mass + 1 / asteroid_physics.mass) / glm::length(asteroid_transform.position)
			)};
			asteroid_physics.velocity = init_velocity_direction * init_velocity;
			auto asteroid_sphere{shape::create_sphere(3, 0.1f)};
			registry.emplace<renderable>(asteroid, asteroid_sphere);
			
			registry.emplace_or_replace<sphere_component>(asteroid, 3, 0.1f);
			registry.emplace<name_component>(asteroid, "ASTEROID ");
		}
	}

	ImGui::InputInt("Asteroid amount", &asteroid_amount);

	ImGui::End();
};

auto world::draw(renderer& renderer, float elapsed_time, float delta_time) const -> void {
	auto view = registry.view<const renderable, transform_component>();
	for (auto&& [entity, renderable, position] : view.each()) {
		renderer.draw_model(renderable.model, {position.position}, elapsed_time, delta_time);
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
