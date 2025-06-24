// =================================================================================
// Filename:    Salix/core/SerializationRegistrations.cpp
// Author:      SalixGameStudio
// Description: Centralized location for Cereal serialization registrations,
//              enabling polymorphic serialization of engine types.
// =================================================================================

#include <cereal/types/polymorphic.hpp>

// We must include the header for every single concrete class we want to register.
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/Sprite2D.h>
#include <Salix/ecs/RenderableElement.h>
// #include <Salix/ecs/AudioSource.h> // Future components would be added here

// --- Registering our polymorphic Element types ---
// This builds the "factory" that allows Cereal to load a Transform or a Sprite2D
// when it's expecting a pointer to the base class, Element.

// First, register the concrete types themselves.

CEREAL_REGISTER_TYPE(Salix::Transform);
CEREAL_REGISTER_TYPE(Salix::Sprite2D);

// Then, register the relationship between the base and derived classes.
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::Transform);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::RenderableElement)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::RenderableElement, Salix::Sprite2D);