#include "model.h"
#include <utility>

namespace gravity {

model::model(std::vector<mesh> const&& meshes) : meshes{std::move(meshes)} {}


}