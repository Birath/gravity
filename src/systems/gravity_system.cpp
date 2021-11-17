#include "gravity_system.h"

#include <glm/gtx/norm.hpp>
#include "components.h"
#include <easy/profiler.h>

namespace gravity::gravity_system {
    


auto update(entt::registry& registry, float delta_time) -> void {
	EASY_FUNCTION();
	// http://www.nssc.ac.cn/wxzygx/weixin/201607/P020160718380095698873.pdf
    auto const& g_c{registry.ctx<const gravity_constant>()};
    auto view = registry.view<transform_component, physics_component>();
	EASY_BLOCK("LOOP", profiler::FORCE_ON);
    for (auto [entity, transform, physics] : view.each()) {
		EASY_BLOCK("Inner loop", profiler::FORCE_ON)
		for (auto [entity_other, transform_other, physics_other] : view.each()) {
			EASY_BLOCK("Inner Loop body", profiler::FORCE_ON);
			if (entity != entity_other) {
				auto const distance{glm::distance2(transform.position, transform_other.position)};
				if (distance < 0.01) continue;
				auto const direction{glm::normalize(transform_other.position - transform.position)};
				auto const force{g_c.value * physics_other.mass / distance * delta_time * direction};
				registry.patch<physics_component>(entity, [force](auto& phy) {phy.velocity += force;});
			}
		}
		auto const velocity{physics.velocity};
		registry.patch<transform_component>(entity, [velocity, delta_time](auto& trans) {
			trans.position += velocity * delta_time;
		});
	}
}
} // namespace gravity::gravity_namespace
