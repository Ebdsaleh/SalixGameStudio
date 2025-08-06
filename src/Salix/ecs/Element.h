// Salix/ecs/Element.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/SimpleGuid.h>
#include <cereal/cereal.hpp>

namespace Salix {
    // Forward declares Entity to avoid circular dependencies
    class Entity;
    class AssetManager;

    class SALIX_API Element {
        public:
            Element() : id(SimpleGuid::generate()) {}
            // A virtual destructor is essential for any class with virtual methods
            virtual ~Element() = default;
            // NEW: Add a pure virtual function to get the element's type name.
            // Every concrete element (Transform, Camera, etc.) MUST implement this.
            virtual const char* get_class_name() const = 0;

            // These are the lifecycle methods that the Entity will call.
            // They are virtual so that concrete elements can override them.
            virtual void on_load(const InitContext& context) {(void) context.asset_manager;}  // Useful for RenderableElement - types.
            virtual void initialize() {}
            virtual void update(float /*delta_time*/) {}
            virtual void shutdown() {}
            Entity* get_owner() { return owner; }
            void set_owner(Entity* owner_entity) {
                owner = owner_entity;
            }
            
            const SimpleGuid&  get_id() const {return id;}
            // This helper returns a direct pointer to the ID's raw value,
            // which is exactly what the reflection system needs.
            const uint64_t* get_id_as_ptr() const { return &id.id; }

            template <class Archive>
            void serialize(Archive& archive) {
                archive(cereal::make_nvp("id", id));
                /// The base class of the hierarchy has no parent to serialize.
                // Since it has no data of its own, this function is empty.
                // It MUST exist, however, to complete the polymorphic chain.
            }
        protected:
            // A pointer to the Entity that owns this element.
            // This allowse elements to communicate with each other.
            Entity* owner = nullptr;
            SimpleGuid id;
        private:
            // The Entity class that will need to be able to set the owner.
            friend class Entity;
    };
} // namespace Salix 