// Salix/ecs/CppScript.h

#pragma once
#include <Salix/core/Core.h>
#include <Salix/ecs/ScriptElement.h>
#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>

namespace Salix {
    class SALIX_API CppScript : public ScriptElement {
    public:
        // We need a virtual destructor for polymorphic types
        virtual ~CppScript() = default;

        // Public method to get the script's unique registered name
        const std::string& get_script_name() const;

    protected:
        // The name this script was registered with in the ScriptFactory
        std::string script_name;
    
    private:
        // Grant Cereal access to our private serialize function
        friend class cereal::access;

        // This function will be called by Cereal to save the script's name.
        template <class Archive>
        void serialize(Archive& archive);
    };
}  // namespace salix