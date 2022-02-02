#ifndef SHADER_H
#define SHADER_H

#include "opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

namespace gravity {

class shader_program {
public:
	shader_program(shader_program const&) = delete;
	auto operator=(shader_program const&) -> shader_program = delete;
	shader_program(shader_program&&) = delete;
	auto operator=(shader_program&&) -> shader_program& = delete;
	shader_program();
	explicit shader_program(std::filesystem::path const& vert_path, std::filesystem::path const& frag_path);

	~shader_program() noexcept;
	auto load_compute_shader(std::filesystem::path const& path) -> void;
	auto use() const -> void;

	template <typename T>
	auto upload_uniform(std::string const& name, T const& value) const -> void {
		auto location = glGetUniformLocation(handle, name.c_str());
		if constexpr (std::is_integral_v<T>) {
			glUniform1i(glGetUniformLocation(handle, name.c_str()), value);
		} else if constexpr (std::is_floating_point_v<T>) {
			glUniform1f(location, value);
		}
	}

	template <>
	auto upload_uniform(std::string const& name, glm::mat4 const& value) const -> void {
		auto const location = glGetUniformLocation(handle, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	template <>
	auto upload_uniform(std::string const& name, glm::vec3 const& value) const -> void {
		glUniform3fv(glGetUniformLocation(handle, name.c_str()), 1, glm::value_ptr(value));
	}
	

private:
	auto load_shader(std::filesystem::path const& file_name, int shader_type) const -> GLuint;
	auto print_shader_info(GLuint shader, std::filesystem::path const& file_name) const -> void;
	auto print_info() const -> void;
	std::filesystem::path vertex_path{};
	std::filesystem::path fragment_path{};
	GLuint handle{0};
	GLuint vertex_shader{0};
	GLuint fragment_shader{0};
	GLuint compute_shader{0};
};

} // namespace gravity

#endif