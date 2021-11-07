#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>

namespace gravity {

struct vertex {
    glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

class mesh {
public:
    mesh(std::vector<unsigned int> const&& indicies, std::vector<vertex> const&& vertices);

    auto generate_buffer() -> void;

    auto size() const -> size_t {
        return indices.size();
    }

    unsigned int vao{};
    unsigned int vbo{};
    unsigned int ebo{};

	std::vector<unsigned int> indices{};
	std::vector<vertex> vertices{};
private:
};

} // namespace gravity::resources

#endif