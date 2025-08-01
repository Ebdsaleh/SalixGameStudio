// Salix/core/SerializationRegistrations.h
#pragma once

// --- Step 1: Include Archives (Essential) ---
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

// --- Step 2: Include Cereal's Polymorphic Type Support ---
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

// --- Step 3: Include All Your Serializable Engine Class Headers ---
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/RenderableElement2D.h>
#include <Salix/ecs/RenderableElement3D.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/ScriptElement.h>
#include <Salix/ecs/CppScript.h> // The engine knows about the CppScript base class
#include <Salix/scripting/ScriptFactory.h>

// --- Step 4: Register Your Engine's Concrete Types ---
CEREAL_REGISTER_TYPE(Salix::Transform);
CEREAL_REGISTER_TYPE(Salix::Sprite2D);
CEREAL_REGISTER_TYPE(Salix::ScriptElement);
CEREAL_REGISTER_TYPE(Salix::CppScript); // Register the CppScript base class

// --- Step 5: Define Polymorphic Relationships for Engine Types ---
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::Transform);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::RenderableElement);
// 2D and 3D elements inherit from the base RenderableElement
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::RenderableElement, Salix::RenderableElement2D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::RenderableElement, Salix::RenderableElement3D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::RenderableElement2D, Salix::Sprite2D);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::ScriptElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::ScriptElement, Salix::CppScript);

// --- Step 6: Provide Custom Save/Load Logic for CppScript ---
// We are "re-opening" the cereal namespace to add our custom functions.
namespace cereal
{
    // This custom "save" function will be used whenever Cereal encounters a CppScript.
    template <class Archive>
    void save(Archive& archive, const Salix::CppScript& script)
    {
        // We save the script's unique registered name as a string.
        archive(cereal::make_nvp("script_name", script.get_script_name()));
    }

    // This custom "load_and_construct" function tells Cereal how to create a script object.
    template <class Archive>
    void load_and_construct(Archive& archive, cereal::construct<Salix::CppScript>& construct)
    {
        // First, read the script's name from the file.
        std::string script_name;
        archive(cereal::make_nvp("script_name", script_name));

        // Use our ScriptFactory to create the correct derived script type (e.g., PlayerMovement).
        // The factory returns a unique_ptr to the base ScriptElement.
        std::unique_ptr<Salix::ScriptElement> script_ptr = Salix::ScriptFactory::get().create_script(script_name);

        if (script_ptr)
        {
            // If creation was successful, give the raw pointer to Cereal's 'construct' helper.
            // We must cast it to the type Cereal is expecting (CppScript*).
            // Cereal will then correctly manage the object's memory.
            construct(dynamic_cast<Salix::CppScript*>(script_ptr.release()));
        }
        else
        {
            // If the script wasn't found in the factory, construct a null pointer.
            construct(nullptr);
        }
    }
} // namespace cereal
