#include "camera.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
namespace gravity {

camera::camera(float fov, float aspect, float znear, float zfar)
	: fov{fov}
	, aspect{aspect}
	, znear{znear}
	, zfar{zfar} {
	update_projection();
}

[[nodiscard]] auto camera::get_projection() const -> glm::mat4 const& {
	return projection;
}

auto camera::set_fov(float f) -> void {
	fov = f;
	update_projection();
}

auto camera::set_aspect_ratio(int width, int height) -> void {
	set_aspect_ratio(static_cast<float>(width) / static_cast<float>(height));
}

auto camera::set_aspect_ratio(float asp) -> void {
	aspect = asp;
	update_projection();
}

auto camera::update_projection() -> void {
	projection = glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

} // namespace water
