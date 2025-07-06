// Salix/scripting/ScriptRegistration.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/SerializationRegistrations.h>
#include <Salix/scripting/ScriptFactory.h>
#include <cereal/types/polymorphic.hpp>
#include <memory>

// --- Registers with Cereal ---
#define REGISTER_SCRIPT_TYPE(T) \
    CEREAL_REGISTER_TYPE(T); \
    CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::CppScript, T);

// --- Registers with the ScriptFactory ---
#define REGISTER_SCRIPT_FACTORY(T) \
    namespace { \
        struct AutoRegister_##T { \
            AutoRegister_##T() { \
                Salix::ScriptFactory::get().register_script(#T, []() { \
                    return std::make_unique<T>(); \
                }); \
            } \
        }; \
        static AutoRegister_##T g_auto_register_##T; \
    }



// ------------------------------------------------------------------------------------
// SCRIPT_SERIALIZATION_BODY(T)
// ------------------------------------------------------------------------------------
// Adds the basic serialize method (archiving base class only)
// ------------------------------------------------------------------------------------

#define SCRIPT_SERIALIZATION_BODY(T) \
private: \
    friend class cereal::access; \
    template <class Archive> \
    void serialize(Archive& archive) { \
        archive(cereal::base_class<Salix::CppScript>(this)); \
    }
