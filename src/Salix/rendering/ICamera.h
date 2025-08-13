#pragma once
#include <Salix/core/Core.h>
#include <glm/glm.hpp>


namespace Salix {
   
    class Transform;

    enum class ProjectionMode {
        Perspective,
        Orthographic
    };  
    

    // An abstract interface that defines what the renderer needs from any camera.
    class SALIX_API ICamera {
    public:
        
        virtual ~ICamera() = default;

        // The "contract": any class that implements ICamera MUST provide these functions.
        virtual const glm::mat4& get_view_matrix() = 0;
        virtual const glm::mat4& get_projection_matrix() = 0;
        virtual const ProjectionMode& get_projection_mode() const = 0;
        virtual void set_projection_mode(ProjectionMode mode) = 0;
        virtual void set_orthographic_size(float size) = 0;
        virtual void set_2D_mode(bool is_2d) { (void) is_2d;}
        virtual Transform* get_transform() { return nullptr; }
    };

}