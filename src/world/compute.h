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
	auto regenerate_buffer(std::vector<T> const& buffer, unsigned int handle) -> void {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
		if (buffer.size() * sizeof(T) > buffer_size()) {
			fmt::print("Regenerating buffer {}\n", handle);
			glBufferData(GL_SHADER_STORAGE_BUFFER, buffer.size() * sizeof(T), &buffer[0], GL_DYNAMIC_READ);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	// Size of the currently bound buffer
	auto buffer_size() -> size_t;

	template <typename T>
	[[nodiscard]] auto upload(std::vector<T> const& buffer, unsigned int handle) -> bool {
        if (buffer.empty()) return false;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
		if (buffer.size() * sizeof(T) > buffer_size()) {
			fmt::print("Buffer to upload is larger than storage buffer {}\n", handle);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			return false;
		} else {
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.size() * sizeof(T), &buffer[0]);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		return glGetError() != GL_NO_ERROR;
	}

	template <typename T>
	[[nodiscard]] auto read(std::vector<T>& buffer, unsigned int handle) -> bool {
        if (buffer.empty()) return false;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.size() * sizeof(T), &buffer[0]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		return glGetError() != GL_NO_ERROR;
	}

	auto dispatch(unsigned int x, unsigned int y, unsigned int z) -> void;
};
} // namespace gravity

#endif