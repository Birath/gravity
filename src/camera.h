#pragma once

#include <glm/mat4x4.hpp>

namespace gravity {

class camera {
public:
	camera(float fov, float aspect, float near, float far);

	[[nodiscard]] auto get_projection() const -> glm::mat4 const&;

	auto set_fov(float fov) -> void;
	auto set_aspect_ratio(float aspect) -> void;
	auto set_aspect_ratio(int width, int height) -> void;

private:
	float fov;
	float aspect;
	float znear;
	float zfar;
	glm::mat4 projection{};

	void update_projection();
};

} // namespace water
