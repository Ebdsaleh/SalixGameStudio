// Entity.h
#pragma once

#include "Element.h"
#include <vector>
#include <memory>  // For std::unique_ptr
#include <iostream>  // For std::cerr

class Entity {
    public:
        Entity() = default;

        // The Entity's lifecycle methods. It will delegate these calls to its elements
        void update(float delta_time) {
            for (auto& element : elements) {
                element->update(delta_time);
            }
        }

        void render(class IRenderer* renderer) {
            for (auto& element : elements) {
                element->render(renderer);
            }
        }
        // --- Template methods for adding and getting elements ---
        // Template methods are defined of type T, adds it to this entity, and returns a pointer to it.
        template<typename T>
        T* add_element() {
            // Create a new element using a smart pointer for safe memory management.
            // std::make_unique is the modern C++ way to create a unique_ptr.
            auto new_element = std::make_unique<T>();

            // Set the owner so the element knows whom it belongs to.
            new_element->owner = this;

            // Get a raw pointer to the new element before we move to a unique_ptr.
            T* element_ptr = new_element.get();

            // Move the unique_ptr into our vector. The vector now owns the memory.
            elements.push_back(std::move(new_element));

            element_ptr->initalize();
            return element_ptr;
        }

        // Searches for an element of type T and returns a pointer to it, or a nullptr if not found.
        template<typename T>
        T* get_element() {
            for (auto& element : elements) {
                // dynamic_cast is a safe way to check if an element is of a certain type.
                T* result = dynamic_cast<T*>(element.get());
                if (result != nullptr) {
                    return result;
                }
            }
            return nullptr;
        }

        // --- Purge functionality ---
        void purge() {
            is_purged_flag = true;
        }

        bool is_purged() const {
            return is_purged_flag;
        }

    private:
        bool is_purged_flag = false;

        // The Entity OWNS its Elements. Using std::unique_ptr ensures that when an Entity is destroyed, 
        // all of its Elements are automatically deleted as well.
        // Implementing the cascading purge system as intended.
        std::vector<std::unique_ptr<Element>> elements;
};