#ifndef COMPUTE_H
#define COMPUTE_H

#include "shader.h"

#include <filesystem>
#include <unordered_map>
#include <fmt/core.h>

namespace gravity {

template <typename T>
struct compute_vec3 {
	T x, y, z, w;
};

class compute {
	shader_program compute_shader{};
	std::unordered_map<unsigned int, unsigned int> handles_to_bindings{};

public:
	explicit compute(std::filesystem::path const& source);

	auto generate_buffer(size_t size, unsigned int binding, GLenum usage) -> unsigned int;
	
	template<typename T>
	auto upload_uniform(std::string const& name, T const& value) -> void {
		compute_shader.upload_uniform(name, value);
	}

	template<typename T>
	auto regenerate_buffer(std::vector<T> const& buffer, unsigned int handle) -> void {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
		GLint usage;
		glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_USAGE, &usage);
		if (buffer.size() * sizeof(T) > buffer_size()) {
			fmt::print("Regenerating buffer {}\n", handle);
			glBufferData(GL_SHADER_STORAGE_BUFFER, buffer.size() * sizeof(T), &buffer[0], usage);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	auto bind_buffer(unsigned int handle, unsigned int binding) -> void;

	// Size of the currently bound buffer
	auto buffer_size() const -> size_t;

	auto clear_buffer(unsigned int handle) -> void;

	auto use() -> void;
	template <typename T>
	auto upload(std::vector<T> const& buffer, unsigned int handle) -> void {
        if (buffer.empty()) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
		if (buffer.size() * sizeof(T) > buffer_size()) {
			fmt::print("Buffer to upload is larger than storage buffer {}\n", handle);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			return;
		} else {
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.size() * sizeof(T), &buffer[0]);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	template <typename T>
	auto read(std::vector<T>& buffer, unsigned int handle) -> void {
        if (buffer.empty()) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.size() * sizeof(T), &buffer[0]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	auto dispatch(unsigned int x, unsigned int y, unsigned int z) -> void;
};
} // namespace gravity

#endif