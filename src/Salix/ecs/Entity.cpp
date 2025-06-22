// =================================================================================
// Filename:    Salix/ecs/Entity.cpp
// Author:      SalixGameStudio
// Description: Implements the Entity class using the PIMPL idiom.
// =================================================================================
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Transform.h>

namespace Salix {

    // Define the hidden implementation details here.
    struct Entity::Pimpl {
        std::string name;
        bool is_purged_flag = false;
        std::vector<std::unique_ptr<Element>> all_elements;
        std::vector<RenderableElement*> renderable_elements;
        Transform* transform = nullptr;
    };

    // --- Constructor and Destructor ---
    Entity::Entity() : pimpl(std::make_unique<Pimpl>()) {
        // Automatically add and store a pointer to the mandatory Transform component.
        pimpl->transform = add_element<Transform>();
    }
    Entity::~Entity() = default;


    // --- Public Methods (now access pimpl) ---
    void Entity::update(float delta_time) {
        for (auto& element : pimpl->all_elements) {
            element->update(delta_time);
        }
    }

    void Entity::render(IRenderer* renderer) {
        for (auto& element : pimpl->renderable_elements) {
            element->render(renderer);
        }
    }

    Transform* Entity::get_transform() const {
        return pimpl->transform;
    }

    void Entity::purge() {
        pimpl->is_purged_flag = true;
    }

    bool Entity::is_purged() const {
        return pimpl->is_purged_flag;
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

    void Entity::set_name(const std::string& new_name) {
        pimpl->name = new_name;
    }

    const std::string& Entity::get_name() const {
        return pimpl->name;
    }

} // namespace Salix