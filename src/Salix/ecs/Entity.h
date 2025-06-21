// =================================================================================
// Filename:    Salix/ecs/Entity.h
// Author:      SalixGameStudio
// Description: Declares the Entity class, a container for Elements.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>
#include <vector>
#include <memory>
#include <typeinfo> // For the get_element helper

namespace Salix {
    // Forward declarations
    class Element;
    class RenderableElement;
    class Transform;
    class IRenderer;

    class SALIX_API Entity{
    public:
        Entity();
        ~Entity(); // Destructor MUST be in the header

        void update(float delta_time);
        void render(IRenderer* renderer);
        Transform* get_transform() const;

        void purge();
        bool is_purged() const;

        // --- PUBLIC TEMPLATE METHODS (defined in the header) ---

        template<typename T>
        T* add_element() {
            // Create the new element safely.
            auto new_element_owner = std::make_unique<T>();
            T* raw_ptr = new_element_owner.get();
            raw_ptr->owner = this;

            // Call the private, non-templated helper to do the real work.
            add_element_internal(std::move(new_element_owner));

            raw_ptr->initialize();
            return raw_ptr;
        }

        template<typename T>
        T* get_element() {
            // Call the private, non-templated helper to do the real work.
            // We pass the typeid so the helper knows what to look for.
            return static_cast<T*>(get_element_internal(typeid(T)));
        }

    private:
        // --- PRIVATE HELPER FUNCTIONS (implemented in .cpp) ---
        void add_element_internal(std::unique_ptr<Element> element);
        Element* get_element_internal(const std::type_info& type_info);

        // --- PIMPL POINTER ---
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };
} // namespace Salix