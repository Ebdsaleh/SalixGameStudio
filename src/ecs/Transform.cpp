// Transform.cpp
#include "Transform.h"

Transform::Transform() :
    parent(nullptr)
    {
        // Default scale is 1, so objects appear at thier normal size.
        scale = { 1.0f, 1.0f, 1.0f};
    }

Transform::~Transform() {
    // When a Transform is destroyed it must detatch itself from its parent.
    if (parent) {
        parent->remove_child(this);
    }

    // It must also orphan all of its children.
    for (Transform* child : children) {
        child->parent = nullptr;
    }
}

void Transform::set_parent(Transform* new_parent) {
    // If we already have a parent, detatch from it first.
    if (parent) {
        parent->remove_child(this);
    }
    // Set the new parent.
    parent = new_parent;

    // if the new parent is not null, add ourselves to its list of children.
    if (parent) {
        parent->add_child(this);
    }
}

Transform* Transform::get_parent() const{
    return parent;
}

void Transform::add_child(Transform* child) {
    // Just add the child to our vector.
    // In a real engine we'd check to make sure it's not already there.
    children.push_back(child);
}

void Transform::remove_child(Transform* child) {
    // Find the child if in our vector and remove it.
    // This is a bit complex in C++, but it's a standard algorithm.
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (*it == child) {
            children.erase(it);
            return;
        }
    }
}

Vector3 Transform::get_world_position() {
    if (parent) {
        // recursively add our local postiton to our parent's world position.
        // NOTE: This is a simplified version. Generally we would use Matrix math.
        return parent->get_world_position() + position;
    }
    // If no parent, return local position Vector3
    return position;
}

Vector3 Transform::get_world_rotation() {
    if (parent) {
        // Rotations also add up.
        return parent->get_world_rotation() + rotation;

    }
    // If no parent, return local rotation Vector3
    return rotation;
}

Vector3 Transform::get_world_scale() {
    if (parent) {
        // Scales multiply
        return parent->get_world_scale() * scale;
    }
    // If no parent, return local scale Vector3
    return scale;
}


