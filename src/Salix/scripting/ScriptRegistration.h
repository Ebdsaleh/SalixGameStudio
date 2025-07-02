// Salix/scripting/ScriptRegistration.h
#pragma once

#include <Salix/scripting/ScriptFactory.h>
#include <cereal/types/polymorphic.hpp>
#include <memory> // For std::make_unique

// Salix/scripting/ScriptRegistration.h
#pragma once

#include <Salix/scripting/ScriptFactory.h>
#include <cereal/types/polymorphic.hpp>
#include <memory> // For std::make_unique

// Move Cereal registration macros outside the constructor.
// These macros need to be in the global or namespace scope.
#define REGISTER_SCRIPT(T) \
    struct Registerer_##T { \
        Registerer_##T() { \
            Salix::ScriptFactory::get().register_script(#T, []() { \
                return std::make_unique<T>(); \
            }); \
        } \
    }; \
    static Registerer_##T g_registerer_##T; \
    //CEREAL_REGISTER_TYPE(T); \
    // CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::CppScript, T); \
    // CEREAL_REGISTER_DYNAMIC_INIT(T);

#define SCRIPT_SERIALIZATION_BODY(T) \
private: \
    friend class cereal::access; \
    template <class Archive> \
    void serialize(Archive& archive) { \
        archive(cereal::base_class<Salix::CppScript>(this)); \
    }