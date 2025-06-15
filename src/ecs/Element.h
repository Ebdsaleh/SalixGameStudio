// Element.h
#pragma once

// Forward declares Entity to avoid circular dependencies
class Entity;

class Element {
    public:
        // A virtual destructor is essential for any class with virtual methods
        virtual ~Element() = default;
        // These are the lifecycle methods that the Entity will call.
        // They are virtual so that concrete elements can override them.
        virtual void initialize() {}
        virtual void update(float delta_time) {}
        virtual void shutdown() {}

    protected:
        // A pointer to the Entity that owns this element.
        // This allowse elements to communicate with each other.
        Entity* owner = nullptr;

    private:
        // The Entity class that will need to be able to set the owner.
        friend class Entity;
};