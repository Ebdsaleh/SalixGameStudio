// Salix/ecs/Transform.cpp
#include <Salix/ecs/Transform.h>
#include <algorithm>

namespace Salix {
    struct Transform::Pimpl {
        Transform* parent = nullptr;
        std::vector<Transform*> children;
    };
    Transform::Transform() : pimpl(std::make_unique<Pimpl>()) {
        // Default scale is 1, so objects appear at thier normal size.
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
} // namespace Salix