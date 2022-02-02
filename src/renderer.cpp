#include "renderer.h"

#include "shape.h"

#include "opengl.h"

#include <algorithm>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <easy/profiler.h>


namespace gravity {

renderer::renderer()
	: default_shader{std::filesystem::path{"assets/shaders/default.vert"}, std::filesystem::path{"assets/shaders/default.frag"}}
	, instanced_shader{std::filesystem::path{"assets/shaders/instanced.vert"}, std::filesystem::path{"assets/shaders/default.frag"}}
	, width{801}
	, height{601}
	, camera{90, static_cast<float>(width) / height, 0.01f, 1000.f}
	, asteroid_model{shape::create_sphere(3, 0.1f)}
	{
	// glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);
	auto const instance_shader_position_location{instanced_shader.get_attrib_location("position")};
	auto const instance_shader_normal_location{instanced_shader.get_attrib_location("normal")};
	auto const instance_shader_uv_location{instanced_shader.get_attrib_location("uv")};
	auto const instance_shader_model_location{instanced_shader.get_attrib_location("instance_model")};

	instance_shader_mv_location = instanced_shader.get_uniform_location("m.vp");
	default_shader_mvp_location = default_shader.get_uniform_location("m.mvp");

	glGenBuffers(1, &asteroid_instance_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, asteroid_instance_buffer);
	for (auto&& mesh : asteroid_model.meshes) {
		glBindVertexArray(mesh.vao);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), nullptr);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

auto renderer::draw_model(model const& model, glm::vec3 const& position, float elapsed_time, float delta_time) const -> void {
	auto model_matrix{glm::rotate(glm::translate(glm::mat4{rendering_tmp}, position), glm::radians(elapsed_time * 100.f), glm::vec3{0.f, 1.f, 0.f})};

	default_shader.upload_uniform_by_location(default_shader_mvp_location, camera.get_projection() * view * model_matrix);
	for (auto&& mesh : model.meshes) {
		draw_mesh(mesh, elapsed_time, delta_time);
	}
}

auto renderer::draw_asteroid_instanced(std::span<glm::mat4 const> const model_matrices) const -> void {
	(void)rendering_tmp;
	EASY_FUNCTION();

	instanced_shader.use();
	EASY_BLOCK("BUFFER", profiler::FORCE_ON);

	glBindBuffer(GL_ARRAY_BUFFER, asteroid_instance_buffer);
	glBufferData(GL_ARRAY_BUFFER, model_matrices.size_bytes(), model_matrices.data(), GL_STATIC_DRAW);
	
	EASY_BLOCK("DRAW", profiler::FORCE_ON);
	instanced_shader.upload_uniform_by_location(instance_shader_mv_location, camera.get_projection() * view);
	for (auto&& mesh : asteroid_model.meshes) {
		EASY_BLOCK("MESH");
		glBindVertexArray(mesh.vao);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.size()), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(model_matrices.size()));
		glBindVertexArray(0);
		EASY_END_BLOCK;
	}
	default_shader.use();
}

auto renderer::draw_mesh(mesh const& mesh, float elapsed_time, float delta_time) const -> void {
	(void)rendering_tmp;
	(void)elapsed_time;
	(void)delta_time;
	assert(mesh.vao != 0 && "Mesh buffers must be generated before drawing");
	glBindVertexArray(mesh.vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.size()), GL_UNSIGNED_INT, nullptr);
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
