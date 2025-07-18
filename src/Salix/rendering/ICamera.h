#pragma once
#include <Salix/core/Core.h>
#include <glm/glm.hpp>

namespace Salix {
    
    // An abstract interface that defines what the renderer needs from any camera.
    class SALIX_API ICamera {
    public:
        virtual ~ICamera() = default;

        // The "contract": any class that implements ICamera MUST provide these functions.
        virtual const glm::mat4& get_view_matrix() = 0;
        virtual const glm::mat4& get_projection_matrix() = 0;
    };

}