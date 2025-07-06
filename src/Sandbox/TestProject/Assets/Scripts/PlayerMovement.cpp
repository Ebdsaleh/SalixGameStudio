// Sandbox/TestProject/Assets/Scripts/PlayerMovement.cpp
#include "PlayerMovement.h"

#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <iostream>

// --- Required for registration ---
#include <Salix/scripting/ScriptRegistration.h>
// #include <cereal/archives/json.hpp>
// #include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>

PlayerMovement::PlayerMovement() {
    script_name = "PlayerMovement"; // This will match what we registered with ScriptFactory
}

void PlayerMovement::initialize() {
    std::cout << "PlayerMovement script initializing..." << std::endl;

    if (owner) {
        transform = owner->get_element<Salix::Transform>();
    }

    if (!transform) {
        std::cerr << "PlayerMovement Error: Could not find a Transform component on the owner entity." << std::endl;
    }
}

void PlayerMovement::update(float delta_time) {
    if (transform) {
        float speed = 20.0f;
        Salix::Vector3 direction = { 1.0f, 0.0f, 0.0f };
        Salix::Vector3 movement = direction * speed * delta_time;
        transform->translate(movement);
    }
}

void PlayerMovement::shutdown() {
    std::cout << "PlayerMovement script shutting down..." << std::endl;
}



// --- Macro expands to factory + type + polymorphic registration ---
// Register for Cereal serialization
REGISTER_SCRIPT_TYPE(PlayerMovement)

// Register with the runtime script factory
REGISTER_SCRIPT_FACTORY(PlayerMovement)

// Required to support dynamic linking for Cereal
CEREAL_REGISTER_DYNAMIC_INIT(PlayerMovement);