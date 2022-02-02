#include "model.h"
#include <utility>

namespace gravity {

model::model(std::vector<mesh>&& meshes) : meshes{std::move(meshes)} {}


}