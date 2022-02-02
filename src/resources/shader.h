#ifndef SHADER_H
#define SHADER_H

#include "opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <easy/profiler.h>
#include <unordered_map>

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
	auto upload_uniform(std::string const& name, T const& value) -> void {
		auto const location = get_uniform_location(name);
		if constexpr (std::is_integral_v<T>) {
			glUniform1i(location, value);
		} else if constexpr (std::is_floating_point_v<T>) {
			glUniform1f(location, value);
		}
	}

	template <>
	auto upload_uniform(std::string const& name, glm::mat4 const& value)  -> void {
		glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	template <>
	auto upload_uniform(std::string const& name, glm::vec3 const& value) -> void {
		glUniform3fv(get_uniform_location(name), 1, glm::value_ptr(value));
	}

	template <typename T>
	auto upload_uniform_by_location(int location, T const& value) const -> void {
		if constexpr (std::is_integral_v<T>) {
			glUniform1i(location, value);
		} else if constexpr (std::is_floating_point_v<T>) {
			glUniform1f(location, value);
		}
	}
	template <>
	auto upload_uniform_by_location(int location, glm::mat4 const& value) const -> void {
		EASY_FUNCTION();
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	// caches the found location
	[[nodiscard]] auto get_attrib_location(std::string const& name) -> GLint;
	[[nodiscard]] auto get_uniform_location(std::string const& name) -> GLint;
	

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
	std::unordered_map<std::string, GLint> attrib_name_to_location{};
	std::unordered_map<std::string, GLint> uniform_name_to_location{};
};

} // namespace gravity

#endif