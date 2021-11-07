#include "shader.h"

#include <stdexcept>
#include <fstream>
#include <fmt/format.h>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl.h"
namespace gravity {

shader_program::shader_program(std::filesystem::path const& vert_path, std::filesystem::path const& frag_path)
	: vertex_path{vert_path}
	, fragment_path{frag_path}
    , handle{glCreateProgram()} {
        if (handle == 0) {
            throw std::runtime_error{"Failed to initialize shader program"};
        }
        
        vertex_shader = load_shader(vertex_path, GL_VERTEX_SHADER);
        fragment_shader = load_shader(frag_path, GL_FRAGMENT_SHADER);
        glLinkProgram(handle);
	    glUseProgram(handle);
        print_info();
    }

shader_program::~shader_program() {
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(handle);
    vertex_shader = 0;
    fragment_shader = 0;
    handle = 0;
}

auto shader_program::use() const -> void {
    glUseProgram(handle);
}

template<typename T>
auto shader_program::upload_uniform(std::string const& name, T const& value) const -> void {
    if constexpr (std::is_integral_v<T>) {
        glUniform1i(glGetUniformLocation(handle, name.c_str()), value);
    }
    else if constexpr (std::is_floating_point_v<T>) {
        glUniform1f(glGetUniformLocation(handle, name));
    }
}

template<>
auto shader_program::upload_uniform(std::string const& name, glm::mat4 const& value) const -> void {
    glUniformMatrix4fv(glGetUniformLocation(handle, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

template<>
auto shader_program::upload_uniform(std::string const& name, glm::vec3 const& value) const -> void {
    glUniform3fv(glGetUniformLocation(handle, name.c_str()), 1, glm::value_ptr(value));
}

auto shader_program::load_shader(std::filesystem::path const& file_name, int shader_type) const -> GLuint {
    GLuint shader_id{glCreateShader(shader_type)};
        if (shader_id == 0) {
            throw std::runtime_error{"Failed to intialize fragment shader"};
        }
        std::ifstream shader_file{file_name};
        if (!shader_file || !shader_file.is_open()) {
            throw std::runtime_error{fmt::format("Failed to read fragment shader {}", file_name.string())};
        }
        std::string const data{(std::istreambuf_iterator<char>(shader_file)), std::istreambuf_iterator<char>()};
        auto const source{data.c_str()};
        glShaderSource(shader_id, 1, &source, nullptr);
        glCompileShader(shader_id);
        print_shader_info(shader_id, file_name);
        glAttachShader(handle, shader_id);
        return shader_id;
}

auto shader_program::print_shader_info(GLuint shader, std::filesystem::path const& file_name) const -> void {
    GLint info_log_length{0};
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 2) {
        fmt::print(stderr, "[From {}:]\n", file_name.string());
		std::string info_log{};
        info_log.resize(info_log_length);
		glGetShaderInfoLog(shader, info_log_length, NULL, info_log.data());
        fmt::print(stderr, "{}\n", info_log);
	}
}

auto shader_program::print_info() const -> void {
    
}

} // namespace gravity::resources