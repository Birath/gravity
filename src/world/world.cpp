#include "world.h"

#include "shape.h"

#include <fmt/core.h>
#include <imgui.h>


namespace gravity {

world::world()
	: registry{} {
	auto const test_entity = registry.create();
	registry.emplace_or_replace<position>(test_entity, 0.0, 0.0, 0.0);
	auto const resolution{5};
	auto sphere{shape::create_sphere(resolution)};
	registry.emplace_or_replace<render_component>(test_entity, sphere);
	registry.emplace_or_replace<sphere_component>(test_entity, resolution);
	// auto const face_entity = registry.create();
	// registry.emplace_or_replace<render_component>(face_entity, shape::create_plane());
}

auto world::tick() -> void {
	auto view = registry.view<position>();

	for (auto [entity, position] : view.each()) {
		// fmt::print("Position: {}, {}\n", position.x, position.y);
		position.x++;
	}


}

auto world::update(float elapsed_time, float delta_time) -> void {
	controller.update(elapsed_time, delta_time);
	auto spheres = registry.view<sphere_component, render_component>();

	ImGui::Begin("Spheres");
	for(auto&& [entity, sphere, render_component] : spheres.each()) {
		auto const resolution{sphere.resolution};
		ImGui::SliderScalar("Test", ImGuiDataType_U8, &sphere.resolution, &shape::min_sphere_resolution, &shape::max_sphere_resolution);
		if (sphere.resolution != resolution) {
			// todo destory old buffers
			shape::regenerate_sphere(render_component.model, sphere.resolution);
		}

	}
	ImGui::End();
};

auto world::draw(renderer& renderer, float elapsed_time, float delta_time) const -> void {
	auto view = registry.view<render_component>();
	for (auto&& [entity, render_component] : view.each()) {
		renderer.draw_model(render_component.model, elapsed_time, delta_time);
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
