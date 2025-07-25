#pragma once
#include <Salix/core/Core.h>
#include <imgui/imgui.h> // For ImTextureID and ImVec2

namespace Salix {

    struct SALIX_API IconInfo {
        // The raw GPU texture handle that ImGui::Image() needs.
        ImTextureID texture_id = 0;

        // The size to render the icon (e.g., 16x16).
        ImVec2 size = ImVec2(16, 16);

        // The texture coordinates for using icon sprite sheets.
        // Defaults to (0,0) and (1,1) to use the whole texture.
        ImVec2 uv0 = ImVec2(0, 0);
        ImVec2 uv1 = ImVec2(1, 1);
    };

} // namespace Salix