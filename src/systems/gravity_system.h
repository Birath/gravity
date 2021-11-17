#ifndef GRAVITY_SYSTEM_H
#define GRAVITY_SYSTEM_H

#include <entt/entt.hpp>

namespace gravity::gravity_system {
struct gravity_constant {
    float value;
};

auto update(entt::registry& registry, float delta_time) -> void;

} // namespace gravity::gravity_system

#endif