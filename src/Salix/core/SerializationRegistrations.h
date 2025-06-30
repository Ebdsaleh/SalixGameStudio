#pragma once

// --- Step 1: Include Archives (Essential) ---
// Include every archive type you might use anywhere in your project.
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

// --- Step 2: Include Cereal's Polymorphic Type Support ---
#include <cereal/types/polymorphic.hpp>

// --- Step 3: Include All Your Serializable Class Headers ---
#include <Salix/ecs/Element.h>
#include <Salix/ecs/Transform.h>
#include <Salix/ecs/RenderableElement.h>
#include <Salix/ecs/Sprite2D.h>
// ...add every other class header you need to save/load

// --- Step 4: Add the Registration Macros ---
// Register all concrete types and their inheritance relationships.
CEREAL_REGISTER_TYPE(Salix::Transform);
CEREAL_REGISTER_TYPE(Salix::Sprite2D);

CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::Transform);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::RenderableElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::RenderableElement, Salix::Sprite2D);