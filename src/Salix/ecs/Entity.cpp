// =================================================================================
// Filename:    Salix/ecs/Entity.cpp
// Author:      SalixGameStudio
// Description: Implements the Entity class using the PIMPL idiom.
// =================================================================================
#include <Salix/core/InitContext.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/BoxCollider.h>
#include <Salix/assets/AssetManager.h>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <Salix/core/SerializationRegistrations.h>
namespace Salix {


    // Define the hidden implementation details here.
    struct Entity::Pimpl {
        std::string name;
        Entity* parent = nullptr;
        bool is_purged_flag = false;
        std::vector<Entity*>children;
        std::vector<std::unique_ptr<Element>> all_elements;
        std::vector<RenderableElement*> renderable_elements;
        Transform* transform = nullptr;
        SimpleGuid id = SimpleGuid::generate();
        InitContext context;
        BoxCollider* box_collider = nullptr;
        Pimpl() = default;
        template<class Archive>
        void serialize (Archive & archive) {
            archive(cereal::make_nvp("name", name), cereal::make_nvp("id", id),
            cereal::make_nvp("elements", all_elements) );
        }
    };

    // --- Constructor and Destructor ---
    Entity::Entity() : pimpl(std::make_unique<Pimpl>()) {
        // Automatically add and store a pointer to the mandatory Transform component.
        pimpl->transform = add_element<Transform>();
        pimpl->box_collider = add_element<BoxCollider>();
       
    }
    Entity::~Entity() = default;


    // --- Public Methods (now access pimpl) ---

    // A useful method for re-loading textures and setting Renderable elements to
    // their loaded textures width and height.
    void Entity::on_load(const InitContext& new_context) {
        pimpl->context = new_context;
        // Reset runtime caches before populating them
        pimpl->transform = nullptr;
        pimpl->renderable_elements.clear();

        // For each element that was just deserialized...
        for (auto& element : pimpl->all_elements) {
            if(element) {
                // 1. Re-establish the owner link
                element->set_owner(this);

                // 2. Re-populate the cached raw pointers
                if (auto* t = dynamic_cast<Transform*>(element.get())) {
                    pimpl->transform = t;
                }
                if (auto* r = dynamic_cast<RenderableElement*>(element.get())) {
                    pimpl->renderable_elements.push_back(r);
                }
                // 3. Call the element's one-time setup function. This is where
                // PlayerMovement or any ScriptElement will get its pointer to the Transform.
                element->initialize();
                // 4. Tell the element to load its own assets
                element->on_load(pimpl->context);
            }
        }
    }
    
    void Entity::update(float delta_time) {
        if (pimpl->is_purged_flag) return;
        for (auto& element : pimpl->all_elements) {
            element->update(delta_time);
        }
    }

    void Entity::render(IRenderer* renderer) {
        if (pimpl->is_purged_flag) return;
        for (auto& element : pimpl->renderable_elements) {
            element->render(renderer);
        }
    }

    Transform* Entity::get_transform() const {
        return pimpl->transform;
    }

    void Entity::purge() {
        // 1. First, detach all children from this entity.
        // We make a copy of the children list because `release_from_parent`
        // will modify the original list.
        auto children_copy = pimpl->children;
        for (auto* child : children_copy) {
            if (child) {
                child->release_from_parent();
            }
        }
        
        // 2. Now that all children are detached, we can mark this entity for purging.
        pimpl->is_purged_flag = true;
    }

    bool Entity::is_purged() const {
        return pimpl->is_purged_flag;
    }

    const SimpleGuid& Entity::get_id() const {
        return pimpl->id;
    }

    void Entity::set_id(SimpleGuid id) {
        pimpl->id = SimpleGuid::invalid();
        pimpl->id = id;
    }

    void Entity::report_ids() const {
        std::cout << "Entity: " << pimpl->name << " - ID: " << pimpl->id.get_value() << std::endl;
        if (pimpl->parent) {
            std::cout << "Parent Entity: " << pimpl->parent->get_name() << " - ID: " << pimpl->parent->get_id().get_value() << std::endl;   
        }
        for (auto& element : pimpl->all_elements) {
            std::cout << "Element: " << element->get_class_name() << " - ID: " << element->id.get_value() << std::endl;
        }
        if (!pimpl->children.empty()) {
            for (auto* child_entity : pimpl->children) {
                std::cout << "Child Entity: " << child_entity->get_name() << " - ID: " << child_entity->get_id().get_value() << std::endl;
            }
        }
    }

    Element* Entity::get_element_by_name(const std::string& element_name) {
        Element* found_element = nullptr;
        if (element_name.empty()) return found_element;
        if (pimpl->is_purged_flag) return found_element;
        for (auto& element : pimpl->all_elements) {
            if (element_name == element->get_name()) {
                 return element.get();
                }
        }
        return found_element;  // No Element found by that name.
    }


    Element* Entity::get_element_by_type_name(const std::string& element_type_name) {
        Element* found_element = nullptr;
        if (element_type_name.empty()) return found_element;
        if (pimpl->is_purged_flag) return found_element;
        for (auto& element : pimpl->all_elements) {
            if (element_type_name == element->get_class_name()) {
                 return element.get();
                }
        }
        return nullptr;  // No Element found by that class name.
    }


    std::vector<Element*> Entity::get_elements_by_type_name(const std::string& type_name) {
        std::vector<Element*> found_elements;
        if (type_name.empty()) return found_elements;
        if (pimpl->is_purged_flag) return found_elements;
        for (auto& element : pimpl->all_elements) {
            if (element && std::string(element->get_class_name()) == type_name) {
                found_elements.push_back(element.get());
            }
        }
        return found_elements;
    }
        
    

    Element* Entity::get_element_by_id(SimpleGuid id) {
        if (id == SimpleGuid::invalid()) return nullptr;

        if(pimpl->is_purged_flag) return nullptr;

        if (pimpl->all_elements.size() == 0) return nullptr;

        for (auto& element : pimpl->all_elements) {
            if (element->get_id() == id) return element.get();
        }
        return nullptr;
    }

    void Entity::add_element(Element* element_to_add) {
        if (!element_to_add) {
            return;
        }

        // 1. Set the back-reference to the owner entity.
        element_to_add->set_owner(this);

        // 2. Wrap the raw pointer in a unique_ptr. This is the key step
        //    that correctly transfers ownership of the memory to the Entity.
        std::unique_ptr<Element> element_owner(element_to_add);

        // 3. Call your existing internal helper to add it to the master list.
        add_element_internal(std::move(element_owner));

        // 4. Call the element's one-time setup function.
        element_to_add->initialize();
    }

    // Heirarchial methods

    void Entity::set_parent(Entity* new_parent) {
        if (pimpl->parent == new_parent) return;  // No change if trying to set the current parent.
        if (new_parent == this) return; // No self-parenting.
        
        // Prevent circular hierarchy
        if (new_parent && new_parent->is_child_of(this)) {
            std::cerr << "Cannot create circular parent-child relationship" << std::endl;
            return;
        }

        // 1. Manage the Entity-level hierarchy
        // Remove from current parent's list of children
        if (pimpl->parent) {
            pimpl->parent->remove_child(this);
        }

        // Set internal parent pointer.
        pimpl->parent = new_parent;
        
        // Add to new parent's list of children.
        if (new_parent) {
            new_parent->add_child(this);
        }

        // 2. Delegate the transform update to the new, smarter Transform::set_parent().
        Transform* new_parent_transform = new_parent ? new_parent->get_transform() : nullptr;
        if (pimpl->transform) {
            pimpl->transform->set_parent(new_parent_transform);
        }        
    }



    void Entity::release_from_parent() {
        if (!pimpl->parent) return;

        // 2. Use the cached transform pointer directly, as you pointed out,
        //    to tell our Transform to handle its own release.
        if (pimpl->transform) { //
            pimpl->transform->release_from_parent();
        }

        // 3. Now, manage the Entity-level hierarchy.
        if (pimpl->parent) {
            pimpl->parent->remove_child(this);
        }

        pimpl->parent = nullptr;
    }




    void Entity::add_child(Entity* child) {
        if (child && std::find(pimpl->children.begin(), pimpl->children.end(), child) == pimpl->children.end()) {
            pimpl->children.push_back(child);
        }
    }

    void Entity::remove_child(Entity* child) {
        if (pimpl->children.size() !=0) {
            pimpl->children.erase(
            std::remove(pimpl->children.begin(), pimpl->children.end(), child),
            pimpl->children.end()
            );
        }
    }


    const std::vector<Entity*>& Entity::get_children() const {
        return pimpl->children;
    }


    bool Entity::is_child_of(const Entity* potential_parent) const {
        const Entity* current = pimpl->parent;
        while (current) {
            if (current == potential_parent) return true;
            current = current->pimpl->parent;
        }
        return false;
    }


    Entity* Entity::get_parent() const {
        return pimpl->parent;
    }


    std::vector<Element*> Entity::get_all_elements() {
        // 1. Create a new, empty vector that will hold the raw pointers.
        std::vector<Element*> raw_pointers;
        // 2. To be efficient, we can reserve memory for the vector ahead of time.
        raw_pointers.reserve(pimpl->all_elements.size());

        // 3. Loop through our internal master list of owning unique_ptrs...
        for (const auto& element_owner_ptr : pimpl->all_elements) {
        // 4. ...for each one, call its .get() method to get the raw pointer,
        //    and add that raw pointer to our result vector.
        raw_pointers.push_back(element_owner_ptr.get());
        }

        // 5. Return the completed list of non-owning raw pointers.
        return raw_pointers;
    }


    // --- Private Helper Implementations (can safely access pimpl) ---
    void Entity::add_element_internal(std::unique_ptr<Element> element) {
        // Check if the new element is also a RenderableElement.
        RenderableElement* renderable = dynamic_cast<RenderableElement*>(element.get());
        if (renderable) {
            pimpl->renderable_elements.push_back(renderable);
        }
        // Add the new element to the master list that owns its memory.
        pimpl->all_elements.push_back(std::move(element));
    }

    Element* Entity::get_element_internal(const std::type_info& type_info) {
        for (auto& element : pimpl->all_elements) {
            // Use typeid to check if an element matches the requested type.
            if (typeid(*element) == type_info) {
                return element.get();
            }
        }
        return nullptr;
    }


    const Element* Entity::get_element_internal(const std::type_info& type_info) const {
        for (const auto& element : pimpl->all_elements) {
            if (typeid(*element) == type_info) {
                return element.get();
            }
        }
        return nullptr;
    }

    void Entity::set_name(const std::string& new_name) {
        pimpl->name = new_name;
    }

    const std::string& Entity::get_name() const {
        return pimpl->name;
    }
    
    
    // --- CEREAL IMPLEMENTATION ---

    template<class Archive>
    void Entity::serialize(Archive& archive) {
        archive(
        cereal::make_nvp("name", pimpl->name),
        cereal::make_nvp("id", pimpl->id),
        cereal::make_nvp("elements", pimpl->all_elements)
        );
    }

    template void Entity::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void Entity::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    template void Entity::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Entity::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &); 

} // namespace Salix