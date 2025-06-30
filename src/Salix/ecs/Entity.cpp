// =================================================================================
// Filename:    Salix/ecs/Entity.cpp
// Author:      SalixGameStudio
// Description: Implements the Entity class using the PIMPL idiom.
// =================================================================================
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Element.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Transform.h>
#include <Salix/assets/AssetManager.h>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <Salix/core/SerializationRegistrations.h>
namespace Salix {


    // Define the hidden implementation details here.
    struct Entity::Pimpl {
        std::string name;
        bool is_purged_flag = false;
        std::vector<std::unique_ptr<Element>> all_elements;
        std::vector<RenderableElement*> renderable_elements;
        Transform* transform = nullptr;
        SimpleGuid id = SimpleGuid::generate();

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
       
    }
    Entity::~Entity() = default;


    // --- Public Methods (now access pimpl) ---

    // A useful method for re-loading textures and setting Renderable elements to
    // their loaded textures width and height.
    void Entity::on_load(AssetManager* asset_manager) {
        for (auto& element : pimpl->all_elements) {
            
            if(element) {
                element->owner = this;
                element->on_load(asset_manager);
            }
        }
    }

    
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

    const SimpleGuid& Entity::get_id() const {
        return pimpl->id;
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
            cereal::make_nvp("PimplData", pimpl)
        );
        
    }

    template void Entity::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void Entity::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    template void Entity::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void Entity::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &); 
    

} // namespace Salix