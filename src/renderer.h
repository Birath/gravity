#ifndef RENDERER_H
#define RENDERER_H

#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "camera.h"
namespace gravity {

class renderer {
public:

    renderer();
    
	auto draw_model(model const& model, float elapsed_time, float delta_time) const -> void;
    auto draw_mesh(mesh const& mesh, float elapsed_time, float delta_time) const -> void;

    auto start_renderer(glm::mat4& render_view) -> void;
    bool render_wireframe{false};
private:
    shader_program default_shader;
    camera camera;
    glm::mat4 view{};
};


} // namespace gravity

#endif // RENDERER_H