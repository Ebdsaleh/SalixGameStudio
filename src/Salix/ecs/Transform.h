// Salix/ecs/Transform.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector3.h>
#include <vector>
#include <memory>
#include <cereal/types/polymorphic.hpp>
#include <cereal/access.hpp>

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
            
            
        private:
            
            // All implementation details are hidden behind this single pointer
            struct Pimpl;
            std::unique_ptr<Pimpl>pimpl;
            friend class cereal::access;
            template<class Archive>
            void serialize(Archive& archive);
            /* Explore this later when we get serialize to work.
            template <class Archive>
            void save(Archive& archive) const; // For writing data

            template <class Archive>
            void load(Archive& archive);      // For reading data
            */
            // Private methods called by set_parent and the destructor
            void add_child(Transform* child);
            void remove_child(Transform* child);

    };
    
} // namespace Salix