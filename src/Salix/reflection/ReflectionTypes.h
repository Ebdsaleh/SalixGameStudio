#pragma once
#include <ostream>
#include <variant>
#include <string>
#include <glm/glm.hpp>
#include <Salix/math/Vector2.h>
#include <Salix/math/Vector3.h>
#include <Salix/math/Color.h>
#include <Salix/math/Point.h>
#include <Salix/math/Rect.h>

namespace Salix {
    // A variant to hold any possible type a property could be.
    using PropertyValue = std::variant<
        int, uint64_t, float, bool, std::string, Vector2, Vector3, Color, Point, Rect, glm::mat4
    >;

    inline std::ostream& operator<<(std::ostream& os, const Salix::PropertyValue& value) {
        // std::visit calls the correct lambda based on the type currently held by the variant.
        std::visit([&os](const auto& val) {
            os << val;
        }, value);
        // Return the stream to allow for chaining (e.g., std::cout << val1 << val2;)
        return os;
    }
}