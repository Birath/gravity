#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <string_view>

#include <glm/glm.hpp>
#include "model.h"

namespace gravity {

struct transform_component {
	glm::vec3 position{};
};

struct physics_component {
	glm::vec3 velocity{};
	float mass{};
};

struct renderable {
	model model;
};

struct sphere_component {
	int resolution{1};
	float radius{0.5};
};

struct deletion_component {};
}

struct name_component {
	std::string_view name{};
};

#endif