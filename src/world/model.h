#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <vector>

namespace gravity {

class model {
    std::vector<mesh> meshes{};

public:
    model(std::vector<mesh> const&& meshes);

    auto get_meshes() const -> std::vector<mesh> const&;
    
};
    
} // namespace gravity


#endif