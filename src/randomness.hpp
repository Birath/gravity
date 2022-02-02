#ifndef RANDOMNESS_H
#define RANDOMNESS_H

#include <glm/glm.hpp>
#include <random>
#include <numbers>
#include <cmath>
#include <fmt/core.h>

namespace gravity::random {
template <typename T, class Generator>
[[nodiscard]] auto generate_point_in_sphere(T& radius_distribution, Generator& engine) -> glm::vec3 {
	auto angle_dist = std::uniform_real_distribution(0.0, 1.0);
	auto const u{angle_dist(engine)};
	auto const v{angle_dist(engine)};

    auto const theta {u * 2.0 * std::numbers::pi};
    auto const phi {std::acos(2.0 * v - 1.0)};
    auto const radius {radius_distribution(engine)};
    auto const pos{glm::normalize(glm::vec3{std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi)})};
    return pos * radius;
}
} // namespace gravity::random

#endif