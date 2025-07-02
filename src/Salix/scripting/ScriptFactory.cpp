// Salix/scripting/ScriptFactory.cpp
#include <Salix/scripting/ScriptFactory.h>
#include <Salix/ecs/ScriptElement.h>
#include <iostream>

namespace Salix {

    // This is the standard singleton pattern implementation.
    ScriptFactory& ScriptFactory::get() {
        static ScriptFactory instance;
        return instance;
    }

    void ScriptFactory::register_script(const std::string& name, Creator creator_func) {
        // Add the new script name and its creation function to our map.
        creators_map[name] = creator_func;
        std::cout << "ScriptFactory: Registered script '" << name << "'" << std::endl;
    }

    std::unique_ptr<ScriptElement> ScriptFactory::create_script(const std::string& name) {
        // Find the script by name in our map.
        auto it = creators_map.find(name);
        if (it != creators_map.end()) {
            // If found, call the associated creation function to get a new instance.
            return it->second();
        }

        // If not found, log an error and return null.
        std::cerr << "ScriptFactory Error: Script '" << name << "' not found." << std::endl;
        return nullptr;
    }

} // namespace Salix