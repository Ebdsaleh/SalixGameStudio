// Salix/ecs/Entity.h
#pragma once

#include <Element.h>
#include <RenderableElement.h>  // To allow rendering of ONLY Renderable Elements (This will optimize the rendering routine).
#include <Transform.h>  // We need the full Transform definition now.
#include <vector>
#include <memory>  // For std::unique_ptr.
#include <iostream>  // For std::cerr.

namespace Salix {

    class Entity {
        public:
            // The constructor now automatically adds a Transform and stores a pointer to it.
            Entity() {
                transform = add_element<Transform>();
            }

            // The Entity's lifecycle methods. It will delegate these calls to its elements.
            void update(float delta_time) {
                for (auto& element : all_elements) {
                    element->update(delta_time);
                }
            }

            // The render loop iterates ONLY over the specialized renderable elements list.
            void render(IRenderer* renderer) {
                for (auto& element : renderable_elements) {
                    element->render(renderer);
                }
            }

            // A convenient helper method to get the entity's transform.
            Transform* get_transform() const {
                return transform;
            }

            // --- Template methods for adding and getting elements ---
            // Template methods are defined of type T, adds it to this entity, and returns a pointer to it.
            template<typename T>
            T* add_element() {
                // Create a new element using a smart pointer for safe memory management.
                // std::make_unique is the modern C++ way to create a unique_ptr.
                auto new_element_owner = std::make_unique<T>();

                // Set the owner so the element knows whom it belongs to.
                new_element_owner->owner = this;

                // Get a raw pointer to the new element before we move to a unique_ptr.
                T* raw_ptr = new_element_owner.get();

                // Check if the new element is also a RenderableElement.
                RenderableElement* renderable = dynamic_cast<RenderableElement*>(raw_ptr);
                if (renderable) {
                    // If it is, add its raw pointer to our specialized list.
                    renderable_elements.push_back(renderable);
                }
                // Add the new element to the master list that owns its memory.
                all_elements.push_back(std::move(new_element_owner));

                raw_ptr->initialize();
                return raw_ptr;
            }

            // Searches for an element of type T and returns a pointer to it, or a nullptr if not found.
            template<typename T>
            T* get_element() {
                for (auto& element : all_elements) {
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
            std::vector<std::unique_ptr<Element>> all_elements;

            // A seperate, non-owning list of only elements that can be rendered.
            // optimizing our render loop.
            std::vector<RenderableElement*> renderable_elements;

            // A convenient, non-owned pointer to the required Transform Element.
            Transform* transform;
    };
} // namespace Salix