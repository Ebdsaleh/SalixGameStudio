// Salix/management/RealmManager.h
#pragma once
#include <Salix/core/Core.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace Salix {

// Forward declarations
class Realm;
class IRenderer;
struct InitContext;

class SALIX_API RealmManager {
public:
    RealmManager();
    ~RealmManager();

    void initialize(const InitContext& context);
    void shutdown();

    void update(float delta_time);
    void render(IRenderer* renderer);

    // Loads a realm from a file and adds it to the manager.
    // Returns true on success.
    bool load_realm(const std::string& path);
    // Loads the active realm and returns true on success.
    bool load_active_realm();
    // Creates a new realm.
    Realm* create_realm(const std::string& realm_name, const std::string& realm_path);

    // Saves the currently active realm to its file.
    bool save_active_realm();

    // Sets the active realm by its name (the name is loaded from the file).
    bool set_active_realm(const std::string& name);
    Realm* get_active_realm() const;

    // Get and Set the project root path
    const std::string& get_project_root_path();
    void set_project_root_path (const std::string& root_path);

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};

} // namespace Salix