// Salix/scripting/ScriptFactory.h
#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <functional>
#include <map>
#include <memory>

namespace Salix {

    // Forward declare the base script type
    class ScriptElement;

    class SALIX_API ScriptFactory {
    public:
        // The function type for creating a script. It takes no arguments
        // and returns a unique_ptr to a new ScriptElement.
        using Creator = std::function<std::unique_ptr<ScriptElement>()>;

        // Gets the single, global instance of the factory.
        static ScriptFactory& get();

        // Registers a new script type with the factory.
        void register_script(const std::string& name, Creator creator_func);

        // Creates a new instance of a script by its registered name.
        std::unique_ptr<ScriptElement> create_script(const std::string& name);

    private:
        // The "sign-up sheet" that maps names to creation functions.
        std::map<std::string, Creator> creators_map;
    };

} // namespace Salix