#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "mesh.h"
#include "model.h"
#include "shader.h"

#include <span>
#include <vector>

namespace gravity {

class renderer {
public:
	renderer();

	auto draw_model(model const& model, glm::vec3 const& position, float elapsed_time, float delta_time) const -> void;
	auto draw_model_instanced(model const& model, std::span<glm::mat4 const> model_matrices) const -> void;
	auto draw_mesh(mesh const& mesh, float elapsed_time, float delta_time) const -> void;
	auto start_renderer(glm::mat4& render_view) -> void;

	auto resize(int width, int height) -> void {
		this->width = width;
		this->height = height;
		camera.set_aspect_ratio(width, height);
	}

	bool render_wireframe{false}; // NOLINT

private:
	shader_program default_shader;
	shader_program instanced_shader;
	camera camera;
	glm::mat4 view{};

	int width{800};
	int height{600};
    float rendering_tmp{1.f};
};

} // namespace gravity

#endif // RENDERER_H