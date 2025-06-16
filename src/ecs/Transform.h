// Transform.h
#pragma once

#include "Element.h"
#include "../math/Vector3.h"  // Include our Vector3 struct.
#include <vector>

class Transform : public Element {
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

    private:
        void add_child(Transform* child);
        void remove_child(Transform* child);

        // --- Hierarchy Data ---
        Transform* parent;
        std::vector<Transform*> children;

};
