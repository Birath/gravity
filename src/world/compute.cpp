#include "compute.h"

#include "opengl.h"

#include <fmt/core.h>

namespace gravity {
compute::compute(std::filesystem::path const& path)
	: compute_shader{shader_program{}} {
	compute_shader.load_compute_shader(path);
}

auto compute::generate_buffer(size_t size, unsigned int binding, GLenum usage) -> unsigned int {
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, (void*)nullptr, usage);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return ssbo;
}

auto compute::buffer_size() -> size_t {
	GLint size;
	glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &size);
	return static_cast<size_t>(size);
}

// template <typename T>
// auto compute::upload(std::vector<T> const& buffer, unsigned int handle) -> bool {
// 	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
// 	glBufferSubData(GL_SHADER_SOURCE_LENGTH, 0, buffer.size() * sizeof(T), &buffer[0]);
// 	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
// 	return glGetError() != GL_NO_ERROR;
// }

// template <typename T>
// auto compute::read(std::vector<T>& buffer, unsigned int handle) -> bool {
// 	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
// 	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.size() * sizeof(T), &buffer[0]);
// 	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
// 	return glGetError() != GL_NO_ERROR;
// }

auto compute::dispatch(unsigned int x, unsigned int y, unsigned int z) -> void {
	compute_shader.use();
	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
    
	fmt::print("max global (total) work group counts x:{} y:{} z:{}\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);
    if (x > static_cast<unsigned int>(work_grp_cnt[0])) throw std::runtime_error{fmt::format("x ({}) is larger than maximum work group count ({})", x, work_grp_cnt[0])};
    if (y > static_cast<unsigned int>(work_grp_cnt[1])) throw std::runtime_error{fmt::format("x ({}) is larger than maximum work group count ({})", y, work_grp_cnt[1])};
    if (z > static_cast<unsigned int>(work_grp_cnt[2])) throw std::runtime_error{fmt::format("x ({}) is larger than maximum work group count ({})", z, work_grp_cnt[2])};
	glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

} // namespace gravity
