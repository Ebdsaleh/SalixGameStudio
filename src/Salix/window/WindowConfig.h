// Salix/window/WindowConfig.h
#pragma once
#include <Salix/core/Core.h>
#include <string>

namespace Salix {
    // A simple struct to hold window configuration.
    struct SALIX_API WindowConfig {
        std::string title = "SalixGameStudio";
        int width = 1280;
        int height = 720;
    };
} // namespace Salix