#include "shape.h"

#include <utility>

namespace gravity::shape {

auto create_face(glm::vec3 const normal, unsigned int resolution) -> mesh {
	auto const axis_a{glm::vec3(normal.y, normal.z, normal.x)};
	auto const axis_b{glm::cross(normal, axis_a)};
	auto vertices{std::vector<vertex>{resolution * resolution}};

	auto triangles{std::vector<unsigned int>((resolution - 1) * (resolution - 1) * 6)};

	auto triangle_index{0};

	for (size_t y = 0; y < resolution; y++) {
		for (size_t x = 0; x < resolution; x++) {
			auto const vertex_index{static_cast<unsigned int>(x + y * resolution)};
			auto const t{glm::vec2(x, y) / static_cast<float>(resolution - 1)};
			auto const point{normal + axis_a * (2 * t.x - 1) + axis_b * (2 * t.y - 1)};
			vertices[vertex_index].position = point;
			if (x != resolution - 1 && y != resolution - 1) {
				triangles[triangle_index + 0] = vertex_index;
				triangles[triangle_index + 1] = vertex_index + resolution + 1;
				triangles[triangle_index + 2] = vertex_index + resolution;
				triangles[triangle_index + 3] = vertex_index;
				triangles[triangle_index + 4] = vertex_index + 1;
				triangles[triangle_index + 5] = vertex_index + resolution + 1;
				triangle_index += 6;
			}
		}
	}

	return mesh{std::move(triangles), std::move(vertices)};
}

auto generate_faces(unsigned int resolution) -> std::vector<mesh> {
	if (faces.contains(resolution)) {
		return faces[resolution];
	}
	auto meshes{std::vector<mesh>{}};

	auto normals = std::vector<glm::vec3>{
		{0, 1, 0},  // UP
		{0, -1, 0}, // DOWN
		{1, 0, 0},  // RIGHT
		{-1, 0, 0}, //LEFT
		{0, 0, 1},  // FORWARD
		{0, 0, -1}, // BACKWARD
	};

	for (auto const& normal : normals) {
		auto mesh{create_face(glm::normalize(normal), resolution)};
		meshes.emplace_back(mesh);
	}

	faces[resolution] = meshes;

	return meshes;
}

auto create_plane() -> model {
	std::vector<vertex> vertices{
		{.position = {0.5f, 0.5f, 0.0f}, .normal = {}, .uv = {}},   // top right
		{.position = {0.5f, -0.5f, 0.0f}, .normal = {}, .uv = {}},  // bottom right
		{.position = {-0.5f, -0.5f, 0.0f}, .normal = {}, .uv = {}}, // bottom left
		{.position = {-0.5f, 0.5f, 0.0f}, .normal = {}, .uv = {}},  // top left
	};

	auto indices{std::vector<unsigned int>{0, 1, 3, 1, 2, 3}};
	auto m{mesh{std::move(indices), std::move(vertices)}};
	m.generate_buffer();
	auto meshes{std::vector{m}};
	return model{std::move(meshes)};
}

auto create_sphere(int resolution, float radius) -> model {
	auto sphere_meshes{generate_faces(resolution)};

	for (auto&& mesh : sphere_meshes) {
		for (auto&& ver : mesh.vertices) {
			ver.position = glm::normalize(ver.position) * radius;
		}
		mesh.generate_buffer();
	}

	return model{std::move(sphere_meshes)};
}

auto regenerate_sphere(model& sphere, int new_resolution, float radius) -> void {
	auto new_meshes{generate_faces(new_resolution)};
	auto combination{std::vector<std::pair<mesh, mesh>>{}};

	for(size_t i{0}; i < new_meshes.size(); ++i) {
		for (auto&& ver: new_meshes[i].vertices) {
			ver.position = glm::normalize(ver.position) * radius;
		}
		sphere.meshes[i].update_buffer(std::move(new_meshes[i].indices), std::move(new_meshes[i].vertices));
	}
}

} // namespace gravity::shape
