// Salix/ecs/ScriptElement.h

#pragma once
#include <cereal/cereal.hpp>
#include <Salix/ecs/Element.h>

namespace Salix {
    
    // Abstract base class for all user-defined scripts.
    // It inherits the standard Element lifecycle.
    class SALIX_API ScriptElement : public Element {
    public:
        virtual ~ScriptElement() = default;

        // We provide empty virtual overrides for the standard Element lifecycle.
        // Concrete scripts will override these with their game logic.
        void initialize() override {}
        void update(float /*delta_time*/) override {}
        void shutdown() override {}

        template <class Archive>
        void serialize(Archive& archive) {
            // This is the crucial part. It tells Cereal to first serialize
            // our parent class, Element. This continues the chain.
            archive( cereal::base_class<Element>(this) );
        }
    };
}