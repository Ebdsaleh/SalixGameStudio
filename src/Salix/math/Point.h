// Salix/math/Point.h
#pragma once
#include <Salix/core/Core.h>
#include <cereal/cereal.hpp>

namespace Salix {

    struct SALIX_API Point {
        int x = 0;
        int y = 0;
       
        // Implement here due to the simple nature of the object.
        template <class Archive>
        void serialize(Archive &) {
            archive (
                cereal::make_nvp("x", x),
                cereal::make_nvp("y", y)
            );
        }
        
    };
   
} // namespace Salix