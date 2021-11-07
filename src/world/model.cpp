#include "model.h"
#include <utility>

namespace gravity {

model::model(std::vector<mesh> const&& meshes) : meshes{std::move(meshes)} {}

auto model::get_meshes() const -> std::vector<mesh> const& {
    return meshes;
}


}