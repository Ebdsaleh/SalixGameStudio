// Salix/math/Rect.h
#pragma once
#include <Salix/core/Core.h>
#include <cereal/cereal.hpp>

namespace {

    struct SALIX_API Rect {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;

        template <class Archive>
        void serialize(Archive& archive) {
            archive (
                cereal::make_nvp("x", x), cereal::make_nvp("y", y),
                cereal::make_nvp("w", w), cereal::make_nvp("h", h)
            );
        }
    };
} // namespace Salix