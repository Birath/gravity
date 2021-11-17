#ifndef SHADER_H
#define SHADER_H

#include <filesystem>
#include "opengl.h"

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
    auto load_computer_shader(std::filesystem::path const& path) -> void;
    auto use() const -> void;
    template<typename T>
    auto upload_uniform(std::string const& name, T const& value) const -> void;

private:
    auto load_shader(std::filesystem::path const& file_name, int shader_type) const -> GLuint;
    auto print_shader_info(GLuint shader, std::filesystem::path const& file_name) const -> void;
    auto print_info() const -> void;
	std::filesystem::path vertex_path{};
	std::filesystem::path fragment_path{};
	GLuint handle{0};
    GLuint vertex_shader{0};
    GLuint fragment_shader{0};
    GLuint computer_shader{0};

};

} // namespace gravity::resources

#endif