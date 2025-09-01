#pragma once
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
}