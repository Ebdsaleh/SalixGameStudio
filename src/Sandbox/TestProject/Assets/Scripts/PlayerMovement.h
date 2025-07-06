// Sandbox/TestProject/Assets/Scripts/PlayerMovement.h
#pragma once

#include <Salix/ecs/CppScript.h>                    // Inherit from CppScript
#include <Salix/scripting/ScriptRegistration.h>     // For SCRIPT_SERIALIZATION_BODY
#include <string>

// Forward declaration to avoid full include
namespace Salix {
    class Transform;
}

class PlayerMovement : public Salix::CppScript {
    SCRIPT_SERIALIZATION_BODY(PlayerMovement)

public:
    PlayerMovement();

    void initialize() override;
    void update(float delta_time) override;
    void shutdown() override;

private:
    Salix::Transform* transform = nullptr;  // Cached pointer to transform
};
