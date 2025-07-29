// Salix/ecs/Transform.h
#pragma once

#include <Salix/core/Core.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector3.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/access.hpp>

namespace Salix {

    class SALIX_API Transform : public Element {
        public:
            Transform();
            virtual ~Transform();

            const char* get_class_name() const override { return "Transform"; }
            // Calculate World Transform
            Vector3 get_world_position();
            Vector3 get_world_rotation();
            Vector3 get_world_scale();

            void set_position(const Vector3& new_position);
            void set_position(const float new_x, float new_y, float new_z);
            
            void set_rotation(const Vector3& new_rotation);
            void set_rotation(const float new_x, float new_y, float new_z);

            void set_scale(const Vector3& new_scale);
            void set_scale(const float new_x, float new_y, float new_z);

            void translate(const Vector3& delta_position);
            void translate(const float new_dp_x, float new_dp_y, float new_dp_z);
            void translate(const glm::vec3& delta_position);

            void rotate(const Vector3& delta_rotation);
            void rotate(const float new_dr_x, float new_dr_y, float new_dr_z);
            void rotate(const glm::vec3& delta_rotation);

            const Vector3& get_position() const;
            const Vector3& get_rotation() const;
            const Vector3& get_scale() const;

            // --- Hierarchy Methods ---
            void set_parent(Transform* new_parent);
            Transform* get_parent() const;
            const std::vector<Transform*>& get_children() const;
            
            glm::vec3 get_forward() const;
            glm::vec3 get_up() const;
            glm::vec3 get_right() const;
            
            glm::mat4 get_model_matrix() const;
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
            // These members are now private.
            // Local transform properties (relative to its parent)
            Vector3 position;
            Vector3 rotation;
            Vector3 scale;
    };
    
} // namespace Salix