// =================================================================================
// Filename:    Salix/ecs/Entity.h
// Author:      SalixGameStudio
// Description: Declares the Entity class, a container for Elements.
// =================================================================================
#pragma once

#include <Salix/core/Core.h>
#include <Salix/core/SimpleGuid.h>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <vector>
#include <memory>
#include <typeinfo> // For the get_element helper
#include <string>

namespace Salix {
    // Forward declarations
    class Element;
    class RenderableElement;
    class Transform;
    class IRenderer;
    class AssetManager;
    struct InitContext;

    class SALIX_API Entity{
        public:
            Entity();
            ~Entity(); // Destructor MUST be in the header

            void on_load(const InitContext& new_context);
            void update(float delta_time);
            void render(IRenderer* renderer);
            Transform* get_transform() const;

            void set_parent(Entity* parent);
            Entity* get_parent() const;
            void release_from_parent();

            void add_child(Entity* child);
            void remove_child(Entity* child);
            const std::vector<Entity*>& get_children() const;

            bool is_child_of(const Entity* potential_parent) const;
            bool is_root() const { return get_parent() == nullptr; }

            void purge();
            bool is_purged() const;
            void set_name(const std::string& new_name);
            const std::string& get_name() const;
            const SimpleGuid&  get_id() const;

            Element* get_element_by_id(SimpleGuid id);
            // --- PUBLIC TEMPLATE METHODS (defined in the header) ---
            // New public template overload for add_element.
            template<typename T>
            void add_element(std::unique_ptr<T> element) {
                // This check ensures you can only add things that inherit from Element.
                static_assert(std::is_base_of<Element, T>::value, "Type T must be derived from Salix::Element");

                if (!element) {
                    return;
                }

                // Get the raw pointer before we move ownership.
                Element* raw_ptr = element.get();
                raw_ptr->set_owner(this);

                // Call the private helper to do the real work.
                // The compiler can now safely convert std::unique_ptr<T> to std::unique_ptr<Element>.
                add_element_internal(std::move(element));

                // Call initialize to complete the setup.
                raw_ptr->initialize();
            }

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
        
            std::vector<Element *> get_all_elements();

            template<typename T>
            T* get_element() {
                // Call the private, non-templated helper to do the real work.
                // We pass the typeid so the helper knows what to look for.
                return static_cast<T*>(get_element_internal(typeid(T)));
            }

            template<typename T>
            const T* get_element() const {
                // Call the private, non-templated helper to do the real work.
                // We pass the typeid so the helper knows what to look for.
                return static_cast<const T*>(get_element_internal(typeid(T)));
            }
            
            template<typename T>
            bool has_element() const {
                // This is now incredibly simple: just call get_element and check the result.
                return get_element<T>() != nullptr;
            }

        private:

            // --- PIMPL POINTER ---
            struct Pimpl;
            std::unique_ptr<Pimpl> pimpl;
            
            // --- The Cereal Friendship ---
            // This gives the Cereal library permission to access our private serialize method.
            friend class cereal::access;

            template<class Archive>
            void serialize(Archive& archive);

            // --- PRIVATE HELPER FUNCTIONS (implemented in .cpp) ---
            void add_element_internal(std::unique_ptr<Element> element);
            Element* get_element_internal(const std::type_info& type_info);
            const Element* get_element_internal(const std::type_info& type_info) const;          
            Entity* parent = nullptr;
            std::vector<Entity*> children;
        };
} // namespace Salix