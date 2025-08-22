// Salix/math/Point.h
#pragma once
#include <Salix/core/Core.h>
#include <cereal/cereal.hpp>
#include <ostream>

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

        
        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }
        bool operator!=(const Point& other) const {
            return !(*this == other);
        }
        
    };

    inline std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "{ x: " << p.x << ", y: " << p.y << " }";
        return os; 
    }
} // namespace Salix