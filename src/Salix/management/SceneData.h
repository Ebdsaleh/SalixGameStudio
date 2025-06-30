// SceneData.h

#pragma once

#include <Salix/core/Core.h>
#include <string>
#include <vector>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/access.hpp>

namespace Salix {

    // You might refine this to also store what *type* of asset it is (texture, model, audio)}
    struct SALIX_API ReferencedAsset {
        std::string path;  // Relative path to the asset
        // enum class AssetType { Texture, Model, Audio, Script, ... };
        // AssetType type; // Optional: if you want to categorize assets

        template <class Archive>
        void serialize(Archive & archive) {
            archive( CEREAL_NVP(path) );
            // if (type) archive(CEREAL_NVP(type)); // If you add an AssetType enum
        }

        ReferencedAsset() = default;
        ~ReferencedAsset() = default;

    };

        // This struct represents the "manifest" or "header" for a scene file.
        // It contains lightweight metadata about the scene.

        struct SceneData {
            std::string scene_name;
            std::string scene_file_path_relative;  // Path to actual '.scene' file.
            size_t entity_count = 0;
            std::string engine_version; // For compatibility checks, e.g., "SalixEngine v1.0"
            std::vector<ReferencedAsset> unique_referenced_assets;

            // Add optional other metadata as needed:
            // std::vector<std::string> required_plugins;
            // std::string author_name;
            // std::string last_modified_date;

            template <class Archive>
            void serialize(Archive& archive) {
                archive(CEREAL_NVP( scene_name ),
                    CEREAL_NVP( scene_file_path_relative ),
                    CEREAL_NVP( entity_count ),
                    CEREAL_NVP( engine_version ),
                    CEREAL_NVP( unique_referenced_assets ));
            }
            SceneData() = default;  // Mandatory for Cereal deserialization
        };


}  // namespace Salix