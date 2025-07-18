// Salix/ecs/Transform.cpp
#include <Salix/ecs/Transform.h>
#include <glm/glm.hpp>
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

namespace Salix {
    struct Transform::Pimpl {
        Transform* parent = nullptr;  // Runtime varaible
        std::vector<Transform*> children;  // Runtime variable
        
        Pimpl() = default;
        template <class Archive>
        void serialize(Archive & archive) {

            // Nothing to serialize here as all Pimpl members are runtime variables.
        }
    };

    Transform::Transform() : pimpl(std::make_unique<Pimpl>()) {
        // Default scale is 1, so objects appear at thier normal size.
        position = { 0.0f, 0.0f, 0.0f };
        rotation = { 0.0f, 0.0f, 0.0f };
        scale = { 1.0f, 1.0f, 1.0f};
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

    void Transform::set_parent(Transform* new_parent) {
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
    }

    Transform* Transform::get_parent() const{
        return pimpl->parent;
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


    Vector3 Transform::get_world_position() {
        if (pimpl->parent) {
            // recursively add our local postiton to our parent's world position.
            // NOTE: This is a simplified version. Generally we would use Matrix math.
            return pimpl->parent->get_world_position() + position;
        }
        // If no parent, return local position Vector3
        return position;
    }

    Vector3 Transform::get_world_rotation() {
        if (pimpl->parent) {
            // Rotations also add up.
            return pimpl->parent->get_world_rotation() + rotation;

        }
        // If no parent, return local rotation Vector3
        return rotation;
    }

    Vector3 Transform::get_world_scale() {
        if (pimpl->parent) {
            // Scales multiply
            return pimpl->parent->get_world_scale() * scale;
        }
        // If no parent, return local scale Vector3
        return scale;
    }
    // --- SETTTERS ---
    // --- POSITION ---
    void Transform::set_position(const Vector3& new_position) {
        position = new_position;
    }
    void Transform::set_position(const float new_x, float new_y, float new_z) {
        position = { new_x, new_y, new_z };
    }
    

    // --- ROTATION ---
    void Transform::set_rotation(const Vector3& new_rotation) {
        rotation = new_rotation;
    }
    void Transform::set_rotation(const float new_x, float new_y, float new_z) {
        rotation = { new_x, new_y, new_z };
    }

    // --- SCALE ---
    void Transform::set_scale(const Vector3& new_scale) {
        scale = new_scale;
    }
    void Transform::set_scale(const float new_x, float new_y, float new_z) {
        scale = { new_x, new_y, new_z };
    }

    // --- TRANSLATORS ---
    void Transform::translate(const Vector3& delta_position) {
        position += delta_position;
    }
    void Transform::translate(const float new_dp_x, float new_dp_y, float new_dp_z) {
        position += { new_dp_x, new_dp_y, new_dp_z };
    }

    void Transform::translate(const glm::vec3& delta_position){
        position.x += delta_position.x;
        position.y += delta_position.y;
        position.z += delta_position.z;
    }

    void Transform::rotate(const Vector3& delta_rotation) {
     rotation += delta_rotation;
    }

    void Transform::rotate(const float new_dr_x, float new_dr_y, float new_dr_z) {
     rotation += { new_dr_x, new_dr_y, new_dr_z};
    }

    void Transform::rotate(const glm::vec3& delta_rotation) {
    rotation.x += delta_rotation.x;
    rotation.y += delta_rotation.y;
    rotation.z += delta_rotation.z;
    }

    const Vector3& Transform::get_position() const {
    return position;
}

    const Vector3& Transform::get_rotation() const {
        return rotation;
    }

    const Vector3& Transform::get_scale() const {
        return scale;
    }


    glm::vec3 Transform::get_forward() const {
        // Rotate the default "forward" vector (0, 0, -1 in OpenGL)
        // by this transform's rotation quaternion.
        
        // 1. Get the rotation angles as a glm::vec3
        glm::vec3 euler_angles = rotation.to_glm(); // 'rotation' is your Vector3 member

        // 2. Create a quaternion from the Euler angles
        glm::quat orientation = glm::quat(euler_angles);

        // 3. Use the quaternion to rotate the direction vector
        return orientation * glm::vec3(0.0f, 0.0f, -1.0f);
    }


    glm::vec3 Transform::get_up() const {
        // Rotate the default "up" vector (0, 1, 0)
        glm::quat orientation = glm::quat(rotation.to_glm());
       
        return orientation * glm::vec3(0.0f, 1.0f, 0.0f);
    }


    glm::vec3 Transform::get_right() const {
        // Rotate the default "right" vector (1, 0, 0)
         glm::quat orientation = glm::quat(rotation.to_glm());
        return orientation * glm::vec3(1.0f, 0.0f, 0.0f);
    }


    template<class Archive>
    void Transform::serialize(Archive& archive) {
        // This is the crucial part. It tells Cereal to first serialize
        // our parent class, Element. This continues the chain.
        archive( cereal::base_class<Element>(this) );

        // The serialize function now accesses the public member directly.
        // The CEREAL_NVP macro creates a named key-value pair, e.g., "position": [x, y, z].
        // Because we already taught Cereal how to handle Vector3, this just works!
        archive(
            cereal::make_nvp("position", position),
            cereal::make_nvp("rotation", rotation),
            cereal::make_nvp("scale", scale)
        );
        
        // NOTE: We will handle serializing the parent/child hierarchy later.
        // That is a more advanced topic involving pointers and object tracking.
        // For now, we are just saving the local transform data.
    }

    template void Transform::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void Transform::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    // Add binary instantiations if needed for scene files (recommended for game data)
    template void Transform::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Transform::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &); 


} // namespace Salix