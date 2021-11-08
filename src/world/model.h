#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <vector>

namespace gravity {

class model {

public:
    model(std::vector<mesh> const&& meshes);

    std::vector<mesh> meshes{};
    
};
    
} // namespace gravity


#endif