// Salix/ecs/Transform.cpp
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <cereal/cereal.hpp>
// If explicitly instantiating JSON archives here
#include <cereal/archives/json.hpp>
// If explicitly instantiating Binary archives
#include <cereal/archives/binary.hpp>
// Required for std::vector in Pimpl
#include <cereal/types/vector.hpp>
// Required for std::unique_ptr<Pimpl>
#include <cereal/types/memory.hpp> 
// Required for cereal::base_class
#include <cereal/types/base_class.hpp> 
#include <Salix/core/SerializationRegistrations.h>
#include <cmath>

namespace Salix {
    struct Transform::Pimpl {
        Transform* parent = nullptr;  // Runtime varaible
        std::vector<Transform*> children;  // Runtime variable
        
        // Local transform properties (relative to its parent)
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
        Pimpl() = default;
        template <class Archive>
        void serialize(Archive & archive) {

            // Nothing to serialize here as all Pimpl members are runtime variables.
        }
    };

    Transform::Transform() : pimpl(std::make_unique<Pimpl>()) {
        // Default scale is 1, so objects appear at thier normal size.
        pimpl->position = { 0.0f, 0.0f, 0.0f };
        pimpl->rotation = { 0.0f, 0.0f, 0.0f };
        pimpl->scale = { 1.0f, 1.0f, 1.0f};
        set_name(get_class_name());
    }

    Transform::~Transform() {
        
        // When a Transform is destroyed it must detatch itself from its parent.
        if (pimpl->parent) {
            pimpl->parent->remove_child(this);
        }

        // It must also orphan all of its children by calling their public API.
        while (!pimpl->children.empty()) {
            pimpl->children.front()->set_parent(nullptr);
        }
            
    }

    void Transform::update(float delta_time) {
        /*
        // This is temporary test code to prove the update loop is working.
        // It will animate the object's X position using a sine wave.
        static float total_time = 0.0f;
        total_time += delta_time;
        set_position(sin(total_time), get_position().y, get_position().z);
        */
}
    void Transform::set_parent(Transform* new_parent) {
        // Prevent circular hierarchy
        if (new_parent && owner) {
            // Check if new_parent is already a child of this transform
            for (Transform* current = new_parent; current != nullptr; current = current->get_parent()) {
                if (current == this) {
                    std::cerr << "Circular transform hierarchy detected" << std::endl;
                    return;
                }
            }
        }
        // 1. Before changing anything, get our current world transform.
        Vector3 world_position = get_world_position();
        Vector3 world_rotation = get_world_rotation();
        Vector3 world_scale    = get_world_scale();  
        
        // 2. Update the transform hierarchy
        // If we already have a parent, detatch from it first.
        if (pimpl->parent) {
            pimpl->parent->remove_child(this);
        }

        // Set the new parent.
        pimpl->parent = new_parent;

        // if the new parent is not null, add ourselves to its list of children.
        if (pimpl->parent) {
            pimpl->parent->add_child(this);
        }

       
        // 3. Calculate and apply the new local transform to preserve the world state.
        if (pimpl->parent) {
            // If we have a new parent, convert the world position to its local space.
            set_position(pimpl->parent->world_to_local_position(world_position));
            set_rotation(world_rotation - pimpl->parent->get_world_rotation());
            set_scale(world_scale / pimpl->parent->get_world_scale());
        } else {
            // If we have no new parent, our local transform IS our world transform.
            set_position(world_position);
            set_rotation(world_rotation);
            set_scale(world_scale);
        }
        
    }

    Transform* Transform::get_parent() const{
        return pimpl->parent;
    }

    

    bool Transform::is_child_of(const Transform* potential_parent) const {
        if (!potential_parent) return false;
        for (Transform* current = pimpl->parent; current != nullptr; current = current->get_parent()) {
            if (current == potential_parent) {
                return true;
            }
        }
        return false;
    }




    const std::vector<Transform*>& Transform::get_children() const {
        return pimpl->children;
    }

    // --- Private Method Implementations ---

    void Transform::add_child(Transform* child) {
        // Check to prevent adding duplicates.
        if (std::find(pimpl->children.begin(), pimpl->children.end(), child) == pimpl->children.end()) {
            pimpl->children.push_back(child);
        }
    }

    void Transform::remove_child(Transform* child) {
        // This is the standard "erase-remove idiom". It's safe and efficient.
        pimpl->children.erase(
            std::remove(pimpl->children.begin(), pimpl->children.end(), child),
            pimpl->children.end()
        );
    }


    void Transform::release_from_parent() {
        if (!pimpl->parent) return;

        // 2. Get our current world-space values BEFORE detaching.
        Vector3 world_position = get_world_position();
        Vector3 world_rotation = get_world_rotation();
        Vector3 world_scale    = get_world_scale();

        // 3. Detach from the parent transform using the existing set_parent method.
        set_parent(nullptr);
        // 4. Apply the stored world values back to our now-local properties.
        set_position(world_position);
        set_rotation(world_rotation);
        set_scale(world_scale);
    }



    Vector3 Transform::get_world_position() const {
        // Get the final world matrix and extract the translation component from the 4th column.
        glm::mat4 world_matrix = get_model_matrix();
        return Vector3(world_matrix[3].x, world_matrix[3].y, world_matrix[3].z);
    }


    Vector3 Transform::get_world_rotation() const {
        
        if (pimpl->parent) {
            return pimpl->parent->get_world_rotation() + pimpl->rotation;
        }
        return pimpl->rotation;
    }


    Vector3 Transform::get_world_scale() const {
        // Decomposing for scale is also complex. Your current multiplication is a good approximation.
        if (pimpl->parent) {
            return pimpl->parent->get_world_scale() * pimpl->scale;
        }
        return pimpl->scale;
    }


    Vector3 Transform::world_to_local_position(const Vector3& world_pos) const {
        if (!pimpl->parent) return world_pos;
        glm::vec4 local = glm::inverse(pimpl->parent->get_model_matrix()) * 
                        glm::vec4(world_pos.x, world_pos.y, world_pos.z, 1.0f);
        return Vector3(local.x, local.y, local.z);
    }


    Vector3 Transform::local_to_world_position(const Vector3& local_pos) const {
        glm::vec4 world = get_model_matrix() * glm::vec4(local_pos.x, local_pos.y, local_pos.z, 1.0f);
        return Vector3(world.x, world.y, world.z);
    }





    // --- SETTTERS ---

    // --- POSITION ---
    void Transform::set_position(const Vector3& new_position) {
        pimpl->position = new_position;
    }
    void Transform::set_position(const float new_x, float new_y, float new_z) {
        pimpl->position = { new_x, new_y, new_z };
    }
    

    // --- ROTATION ---
    void Transform::set_rotation(const Vector3& new_rotation) {
        pimpl->rotation = new_rotation;
    }
    void Transform::set_rotation(const float new_x, float new_y, float new_z) {
        pimpl->rotation = { new_x, new_y, new_z };
    }

    // --- SCALE ---
    void Transform::set_scale(const Vector3& new_scale) {
        pimpl->scale = new_scale;
    }
    void Transform::set_scale(const float new_x, float new_y, float new_z) {
        pimpl->scale = { new_x, new_y, new_z };
    }

    // --- TRANSLATORS ---
    void Transform::translate(const Vector3& delta_position) {
        pimpl->position += delta_position;
    }
    void Transform::translate(const float new_dp_x, float new_dp_y, float new_dp_z) {
        pimpl->position += { new_dp_x, new_dp_y, new_dp_z };
    }

    void Transform::translate(const glm::vec3& delta_position){
        pimpl->position.x += delta_position.x;
        pimpl->position.y += delta_position.y;
        pimpl->position.z += delta_position.z;
    }

    void Transform::rotate(const Vector3& delta_rotation) {
     pimpl->rotation += delta_rotation;
    }

    void Transform::rotate(const float new_dr_x, float new_dr_y, float new_dr_z) {
     pimpl->rotation += { new_dr_x, new_dr_y, new_dr_z};
    }

    void Transform::rotate(const glm::vec3& delta_rotation) {
    pimpl->rotation.x += delta_rotation.x;
    pimpl->rotation.y += delta_rotation.y;
    pimpl->rotation.z += delta_rotation.z;
    }

    const Vector3& Transform::get_position() const {
    return pimpl->position;
}

    const Vector3& Transform::get_rotation() const {
        return pimpl->rotation;
    }

    const Vector3& Transform::get_scale() const {
        return pimpl->scale;
    }


    glm::vec3 Transform::get_forward() const {
        // Rotate the default "forward" vector (0, 0, -1 in OpenGL)
        // by this transform's rotation quaternion.
        
        // 1. Get the rotation angles as a glm::vec3
        glm::vec3 euler_angles = pimpl->rotation.to_glm(); // 'rotation' is your Vector3 member

        // 2. Create a quaternion from the Euler angles
        glm::quat orientation = glm::quat(euler_angles);

        // 3. Use the quaternion to rotate the direction vector
        return orientation * glm::vec3(0.0f, 0.0f, -1.0f);
    }


    glm::vec3 Transform::get_up() const {
        // Rotate the default "up" vector (0, 1, 0)
        glm::quat orientation = glm::quat(pimpl->rotation.to_glm());
       
        return orientation * glm::vec3(0.0f, 1.0f, 0.0f);
    }


    glm::vec3 Transform::get_right() const {
        // Rotate the default "right" vector (1, 0, 0)
         glm::quat orientation = glm::quat(pimpl->rotation.to_glm());
        return orientation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    


    glm::mat4 Transform::get_model_matrix() const {
        // 1. Calculate this transform's local matrix. Your existing code is perfect.
        const glm::mat4 transform_x = glm::rotate(glm::mat4(1.0f), glm::radians(pimpl->rotation.x),
            glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 transform_y = glm::rotate(glm::mat4(1.0f), glm::radians(pimpl->rotation.y),
            glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 transform_z = glm::rotate(glm::mat4(1.0f), glm::radians(pimpl->rotation.z),
            glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::mat4 rotation_matrix = transform_z * transform_y * transform_x;
        const glm::mat4 local_matrix = glm::translate(glm::mat4(1.0f), pimpl->position.to_glm()) *
                                    rotation_matrix *
                                    glm::scale(glm::mat4(1.0f), pimpl->scale.to_glm());

        // 2. If we have a parent, multiply our local matrix by our parent's world matrix.
        if (pimpl->parent) {
            return pimpl->parent->get_model_matrix() * local_matrix;
        }
        
        // 3. If there's no parent, our local matrix is our world matrix.
        return local_matrix;
    }






    // This is the SAVE function. It reads data using your public getters.
    template<class Archive>
    void Transform::save(Archive& archive) const {
        // First, save the base class data.
        archive(cereal::base_class<Element>(this));
        
        // Get the values using your public getters and serialize them.
        archive(
            cereal::make_nvp("position", get_position()),
            cereal::make_nvp("rotation", get_rotation()),
            cereal::make_nvp("scale", get_scale())
        );
    }

    // This is the LOAD function. It writes data using your public setters.
    template<class Archive>
    void Transform::load(Archive& archive) {
        // First, load the base class data.
        archive(cereal::base_class<Element>(this));

        // Create temporary variables to hold the loaded data.
        Vector3 loaded_position;
        Vector3 loaded_rotation;
        Vector3 loaded_scale;

        // Load the data from the archive into the temporary variables.
        archive(
            cereal::make_nvp("position", loaded_position),
            cereal::make_nvp("rotation", loaded_rotation),
            cereal::make_nvp("scale", loaded_scale)
        );
        
        // Use your public setters to apply the loaded data.
        set_position(loaded_position);
        set_rotation(loaded_rotation);
        set_scale(loaded_scale);
    }

    // --- ADD THESE LINES AT THE VERY END OF Transform.cpp ---
    // This is required to make the template functions available to other parts of your code.

    template void Transform::save<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&) const;
    template void Transform::load<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void Transform::save<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&) const;
    template void Transform::load<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);


} // namespace Salix