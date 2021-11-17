#include "renderer.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "opengl.h"

namespace gravity {

renderer::renderer()
	: default_shader{std::filesystem::path{"assets/shaders/default.vert"}, std::filesystem::path{"assets/shaders/default.frag"}}
	, camera{90, static_cast<float>(width)/height, 0.01f, 1000.f} {
		// glEnable(GL_DEPTH_TEST);
		// glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK);
    }

auto renderer::draw_model(model const& model, glm::vec3 const& position, float elapsed_time, float delta_time) const -> void {
	auto model_matrix{glm::rotate(
		glm::translate(glm::mat4{1.f}, position),
		glm::radians(elapsed_time * 100.f),
		glm::vec3{0.f, 1.f, 0.f})
	};
	
	default_shader.upload_uniform("m.mvp", camera.get_projection() * view * model_matrix);
	for (auto &&mesh : model.meshes)
	{
		draw_mesh(mesh, elapsed_time, delta_time);
	}

	// model_matrix = glm::translate(glm::mat4{1}, glm::vec3{0.f, 0.f, 0});
	// default_shader.upload_uniform("m.model", model_matrix);
	// default_shader.upload_uniform("m.mvp", projection * view * model_matrix);
	// for (auto &&mesh : model.get_meshes())
	// {
	// 	draw_mesh(mesh);
	// }
	
}

auto renderer::draw_mesh(mesh const& mesh, float elapsed_time, float delta_time) const -> void {
	(void)elapsed_time;
	(void)delta_time;
	assert(mesh.vao != 0 && "Mesh buffers must be generated before drawing");
	glBindVertexArray(mesh.vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

auto renderer::start_renderer(glm::mat4& render_view) -> void {
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	if (render_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
    default_shader.use();
	view = glm::mat4{render_view};
}

} // namespace gravity
