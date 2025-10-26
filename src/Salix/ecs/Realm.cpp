// Salix/ecs/Realm.cpp
#include <Salix/ecs/Realm.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Camera.h>
#include <Salix/events/EventManager.h>
#include <Salix/events/BeforeEntityPurgedEvent.h>
#include <Salix/core/SerializationRegistrations.h>
#include <Salix/management/FileManager.h>
#include <Salix/core/InitContext.h>
#include <fstream>
#include <cereal/archives/json.hpp>

namespace Salix {

    struct Realm::Pimpl {
        std::string name;
        std::string path;
        std::vector<std::unique_ptr<Entity>> entities;
        ICamera* active_camera = nullptr;
        SimpleGuid main_camera_entity_id = SimpleGuid::invalid();
        InitContext context;
    };

    // Constructors
    Realm::Realm() : pimpl(std::make_unique<Pimpl>()) {
        set_name("New Realm");
        set_path("Undefined");
    }

    Realm::Realm(const std::string& name) : pimpl(std::make_unique<Pimpl>()) {
        set_name(name);
        set_path("Undefined");
    }

    Realm::Realm(const std::string& name, const std::string& path) : pimpl(std::make_unique<Pimpl>()) {
        set_name(name);
        set_path(path);
    }

    Realm::~Realm() = default;

    // Static Factory for Loading
    std::unique_ptr<Realm> Realm::load_from_file(const std::string& path, const InitContext& context) {
        if (!FileManager::path_exists(path)) {
            std::cerr << "Realm Error: File not found at '" << path << "'." << std::endl;
            return nullptr;
        }

        std::ifstream file_stream(path);
        if (!file_stream.is_open()) {
            std::cerr << "Realm Error: Could not open file '" << path << "'." << std::endl;
            return nullptr;
        }

        try {
            cereal::JSONInputArchive archive(file_stream);
            // Create a temporary unique_ptr and deserialize into it
            auto realm = std::make_unique<Realm>("", ""); // Temp name/path
            archive(*realm);
            
            // After loading, run the on_load lifecycle method for all entities
            realm->on_load(context);
            return realm;
        } catch (const cereal::Exception& e) {
            std::cerr << "Realm Error: Failed to deserialize realm from '" << path << "': " << e.what() << std::endl;
            return nullptr;
        }
    }

    // Saving
    bool Realm::save_to_file(const std::string& path) const {
        std::ofstream file_stream(path);
        if (!file_stream.is_open()) {
            std::cerr << "Realm Error: Could not open file for writing: '" << path << "'." << std::endl;
            return false;
        }

        try {
            cereal::JSONOutputArchive archive(file_stream);
            archive(*this);
            return true;
        } catch (const cereal::Exception& e) {
            std::cerr << "Realm Error: Failed to serialize realm to '" << path << "': " << e.what() << std::endl;
            return false;
        }
    }

    // Retrieve name and path
    const std::string& Realm::get_path() {
        return pimpl->path;
    }

    const std::string& Realm::get_name() {
        return pimpl->name;
    }

    // Set name and path
    void Realm::set_path(const std::string& new_path) {
        if (new_path.empty()) return;
        pimpl->path = new_path;
    }

    void Realm::set_name(const std::string& new_name) {
        if (new_name.empty()) return;
        pimpl->name = new_name;
    }

    // Lifecycle
    void Realm::on_load(const InitContext& context) {
        for (auto& entity : pimpl->entities) {
            if (entity) {
                entity->on_load(context);
            }
        }
    }

    void Realm::update(float delta_time) {
        for (auto& entity : pimpl->entities) {
            if (entity && !entity->is_purged()) {
                entity->update(delta_time);
            }
        }
    }

    void Realm::render(IRenderer* renderer) {
        for (auto& entity : pimpl->entities) {
            if (entity && entity->is_visible() && !entity->is_purged()) {
                entity->render(renderer);
            }
        }
    }

    void Realm::maintain() {
        // Dispatch pre-purge events first
        if (pimpl->context.event_manager) {
            for (const auto& entity : pimpl->entities) {
                if (entity && entity->is_purged()) {
                    pimpl->context.event_manager->dispatch(std::make_unique<BeforeEntityPurgedEvent>(entity.get()));
                }
            }
        }

        // Now, erase the purged entities
        auto it = std::remove_if(pimpl->entities.begin(), pimpl->entities.end(), 
            [](const std::unique_ptr<Entity>& entity) {
                return !entity || entity->is_purged();
        });
        pimpl->entities.erase(it, pimpl->entities.end());
    }

    // Asset loading
    void Realm::load_assets(InitContext& context) {
        if (context.asset_manager == nullptr) return;
        on_load(context);
    }

    void Realm::set_context(const InitContext& context) {
        if (context.asset_manager == nullptr) return;
        pimpl->context = context;
    }
    
    // Entity Management
    Entity* Realm::create_entity(const std::string& name) {
        auto new_entity = std::make_unique<Entity>();
        new_entity->set_name(name);
        Entity* ptr = new_entity.get();
        pimpl->entities.push_back(std::move(new_entity));
        return ptr;
    }

    Entity* Realm::create_entity(SimpleGuid id, const std::string& name) {
        auto new_entity = std::make_unique<Entity>();
        new_entity->set_name(name);
        new_entity->set_id(id);
        Entity* ptr = new_entity.get();
        pimpl->entities.push_back(std::move(new_entity));
        return ptr;
    }

    void Realm::clear_all_entities() {
        pimpl->entities.clear();
    }

    // Retrieval
    Entity* Realm::get_entity_by_id(SimpleGuid id) {
        for (const auto& entity : pimpl->entities) {
            if (entity && entity->get_id() == id) {
                return entity.get();
            }
        }
        return nullptr;
    }

    Entity* Realm::get_entity_by_name(const std::string& name) {
        for (const auto& entity : pimpl->entities) {
            if (entity && entity->get_name() == name) {
                return entity.get();
            }
        }
        return nullptr;
    }

    std::vector<Entity*> Realm::get_entities() {
        std::vector<Entity*> raw_pointers;
        raw_pointers.reserve(pimpl->entities.size());
        for (const auto& entity_ptr : pimpl->entities) {
            raw_pointers.push_back(entity_ptr.get());
        }
        return raw_pointers;
    }

    // Camera Management
    ICamera* Realm::get_active_camera() {
        return pimpl->active_camera;
    }

    // Set active Camera
    void Realm::set_active_camera(ICamera* camera) {
        if (!camera) return;
        pimpl->active_camera = camera;
    }

    void Realm::set_main_camera_entity(SimpleGuid entity_id) { pimpl->main_camera_entity_id = entity_id; }
    SimpleGuid Realm::get_main_camera_entity_id() const { return pimpl->main_camera_entity_id; }
    
    void Realm::set_active_camera_entity(SimpleGuid entity_id) {
        // 1. Deactivate the OLD active camera (if one exists).
        if (pimpl->main_camera_entity_id.is_valid()) {
            Entity* old_camera_entity = get_entity_by_id(pimpl->main_camera_entity_id);
            if (old_camera_entity) {
                // Get the general ICamera interface first.
                if (ICamera* i_camera = old_camera_entity->get_element<ICamera>()) {
                    // Safely cast it to the concrete Camera to access activate/deactivate.
                    if (Camera* old_camera = dynamic_cast<Camera*>(i_camera)) {
                        old_camera->deactivate();
                    }
                }
            }
        }

        // 2. Set the NEW active camera's ID.
        pimpl->main_camera_entity_id = entity_id;

        // 3. Activate the NEW camera component (if the new ID is valid).
        if (pimpl->main_camera_entity_id.is_valid()) {
            Entity* new_camera_entity = get_entity_by_id(pimpl->main_camera_entity_id);
            if (new_camera_entity) {
                if (ICamera* i_camera = new_camera_entity->get_element<ICamera>()) {
                    if (Camera* new_camera = dynamic_cast<Camera*>(i_camera)) {
                        set_active_camera(new_camera);
                        new_camera->activate();
                    }
                }
            }
        }
    }
    

    // Serialization
    template <class Archive>
    void Realm::serialize(Archive& archive) {
        archive(
            cereal::make_nvp("name", pimpl->name),
            cereal::make_nvp("path", pimpl->path),
            cereal::make_nvp("main_camera_entity_id", pimpl->main_camera_entity_id),
            cereal::make_nvp("entities", pimpl->entities)
        );
    }

} // namespace Salix