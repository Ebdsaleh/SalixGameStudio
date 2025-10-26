// Salix/management/RealmManager.cpp
#include <Salix/management/RealmManager.h>
#include <Salix/ecs/Realm.h>
#include <Salix/core/InitContext.h>
#include <filesystem>
#include <iostream>

namespace Salix {

    struct RealmManager::Pimpl {
        std::map<std::string, std::unique_ptr<Realm>> realms;
        Realm* active_realm = nullptr;
        InitContext context;
        std::string project_root_path;
    };

    RealmManager::RealmManager() : pimpl(std::make_unique<Pimpl>()) {}
    RealmManager::~RealmManager() = default;

    void RealmManager::initialize(const InitContext& context) {
        pimpl->context = context;
    }

    void RealmManager::shutdown() {
        pimpl->realms.clear();
        pimpl->active_realm = nullptr;
    }

    void RealmManager::update(float delta_time) {
        if (pimpl->active_realm) {
            pimpl->active_realm->update(delta_time);
        }
    }

    void RealmManager::render(IRenderer* renderer) {
        if (pimpl->active_realm) {
            pimpl->active_realm->render(renderer);
        }
    }

    bool RealmManager::load_realm(const std::string& path) {
        std::string realm_name = std::filesystem::path(path).stem().string();

        if (pimpl->realms.count(realm_name)) {
            std::cout << "RealmManager: Realm '" << realm_name << "' is already loaded." << std::endl;
            return true;
        }

        auto loaded_realm = Realm::load_from_file(path, pimpl->context);
        if (loaded_realm) {
            pimpl->realms[realm_name] = std::move(loaded_realm);
            return true;
        }
        return false;
    }

    bool RealmManager::load_active_realm() {
        if (pimpl->active_realm == nullptr || pimpl->context.asset_manager == nullptr) return false;
        pimpl->active_realm->on_load(pimpl->context);
        return true;
    }

    Realm* RealmManager::create_realm(const std::string& realm_name, const std::string& realm_path) {
        if (realm_name.empty() || realm_path.empty()) return nullptr;
        std::unique_ptr<Realm> new_realm;
        new_realm = std::make_unique<Realm>();
        new_realm->set_name(realm_name);
        new_realm->set_path(realm_path);
        pimpl->realms[realm_name] = std::move(new_realm);
        return pimpl->realms[realm_name].get();
    }

    bool RealmManager::save_active_realm() {
        if (!pimpl->active_realm) {
            std::cerr << "RealmManager Error: No active realm to save." << std::endl;
            return false;
        }
        
        // We need to know the path to save to. Let's assume the project manager
        // or another system provides the root path. For now, we'll hardcode it.
        std::string path_to_save = "src/Sandbox/TestProject/" + pimpl->active_realm->get_path();
        return pimpl->active_realm->save_to_file(path_to_save);
    }

    bool RealmManager::set_active_realm(const std::string& name) {
        auto it = pimpl->realms.find(name);
        if (it != pimpl->realms.end()) {
            pimpl->active_realm = it->second.get();
            std::cout << "RealmManager: Active realm set to '" << name << "'." << std::endl;
            return true;
        }
        
        std::cerr << "RealmManager Error: Could not find realm '" << name << "' to set as active." << std::endl;
        return false;
    }

    Realm* RealmManager::get_active_realm() const {
        return pimpl->active_realm;
    }

    const std::string& RealmManager::get_project_root_path() {
        return pimpl->project_root_path;
    }

    void RealmManager::set_project_root_path (const std::string& root_path) {
        if (root_path.empty()) return;
        pimpl->project_root_path = root_path;
    }


} // namespace Salix