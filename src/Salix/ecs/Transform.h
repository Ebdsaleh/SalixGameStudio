// Salix/ecs/Transform.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector3.h>
#include <vector>
#include <memory>
#include <cereal/cereal.hpp>

namespace Salix {

    class SALIX_API Transform : public Element {
        public:
            Transform();
            virtual ~Transform();

            // --- Public Data ---
            // Keeping these public for now, for the sake of simplicity.

            // Local transform properties (relative to its parent)
            Vector3 position;
            Vector3 rotation;
            Vector3 scale;

            // Calculate World Transform
            Vector3 get_world_position();
            Vector3 get_world_rotation();
            Vector3 get_world_scale();

            // --- Hierarchy Methods ---
            void set_parent(Transform* new_parent);
            Transform* get_parent() const;
            const std::vector<Transform*>& get_children() const;
            template<class Archive>
            void serialize(Archive& archive)
            {
                // The CEREAL_NVP macro creates a named key-value pair, e.g., "position": [x, y, z].
                // Because we already taught Cereal how to handle Vector3, this just works!
                archive( CEREAL_NVP(position), CEREAL_NVP(rotation), CEREAL_NVP(scale) );
                
                // NOTE: We will handle serializing the parent/child hierarchy later.
                // That is a more advanced topic involving pointers and object tracking.
                // For now, we are just saving the local transform data.
            }


        private:
            // Private methods called by set_parent and the destructor
            void add_child(Transform* child);
            void remove_child(Transform* child);

            // All implementation details are hidden behind this single pointer
            struct Pimpl;
            std::unique_ptr<Pimpl>pimpl;

    };
    
} // namespace Salix