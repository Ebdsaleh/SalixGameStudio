// Salix/ecs/Realm.h
#pragma once
#include <Salix/core/Core.h>
#include <vector>
#include <memory>
#include <string>
#include <cereal/access.hpp>

namespace Salix {

    // Forward declarations
    class Entity;
    class IRenderer;
    struct InitContext;
    class SimpleGuid;
    class ICamera;

    class SALIX_API Realm {
    public:
        // Constructors
        Realm();
        Realm(const std::string& name);
        Realm(const std::string& name, const std::string& path);
        ~Realm();
        // Factory function to load a realm from a file.
        // This is a cleaner pattern than loading into an existing object.
        static std::unique_ptr<Realm> load_from_file(const std::string& path, const InitContext& context);

        // Saves the current state of the realm to a file.
        bool save_to_file(const std::string& path) const;

        // Retrieve name and path
        const std::string& get_path();
        const std::string& get_name();

        // Set name and path
        void set_path(const std::string& new_path);
        void set_name(const std::string& new_name);

        // Set context
        void set_context(const InitContext& context);

        // Lifecycle methods that are called by the RealmManager
        void on_load(const InitContext& context);
        void update(float delta_time);
        void render(IRenderer* renderer);
        void maintain();

        // Asset loading
        void load_assets(InitContext& context);
        // Entity management
        Entity* create_entity(const std::string& name = "Entity");
        Entity* create_entity(SimpleGuid id, const std::string& name);
        void clear_all_entities();    

        // Entity retrieval
        Entity* get_entity_by_id(SimpleGuid id);
        Entity* get_entity_by_name(const std::string& name);
        std::vector<Entity*> get_entities();

        // Camera management
        void set_main_camera_entity(SimpleGuid entity_id);
        SimpleGuid get_main_camera_entity_id() const;
        ICamera* Realm::get_active_camera();
        void set_active_camera(ICamera* camera);
        void set_active_camera_entity(SimpleGuid entity_id);

    private:
    
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
        
        // Grant access to RealmManager to call the private constructor
        friend class RealmManager; 
        // Grant access to Cereal for serialization
        friend class cereal::access;
        template <class Archive>
        void serialize(Archive& archive);
    };

} // namespace Salix