#ifndef SHAPE_H
#define SHAPE_H

#include "mesh.h"
#include "model.h"

#include <glm/glm.hpp> // vec3, vec2
#include <unordered_map>
#include <vector>

namespace gravity::shape {

auto constexpr  max_sphere_resolution{25};
auto constexpr  min_sphere_resolution{2};


auto create_face(glm::vec3 const normal, unsigned int resolution) -> mesh;

static auto faces{std::unordered_map<unsigned int, std::vector<mesh>>{}};

auto generate_faces(unsigned int resolution) -> std::vector<mesh>;

auto create_plane() -> model;

auto create_sphere(int resolution, float radius) -> model;

auto regenerate_sphere(model& sphere, int new_resolution, float radius) -> void;

} // namespace gravity::shape
#endif