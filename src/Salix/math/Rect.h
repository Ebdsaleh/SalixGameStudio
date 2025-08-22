// Salix/math/Rect.h
#pragma once
#include <Salix/core/Core.h>
#include <cereal/cereal.hpp>
#include <ostream>
#include <type_traits> // ✨ Required for std::is_arithmetic and std::enable_if

namespace Salix {

    template<typename T>
    struct SALIX_API Rect_ {
        T x = 0, y = 0, w = 0, h = 0;

        // Default constructor
        Rect_() = default;

        // NEW GENERIC CONSTRUCTOR
        // This constructor accepts any four numeric types (int, float, double, etc.).
        // The 'enable_if' part is a C++ safety feature that ensures this constructor
        // is ONLY available when you pass in number-like types.
        template<
            typename X, typename Y, typename W, typename H,
            typename = std::enable_if_t<
                std::is_arithmetic_v<X> && std::is_arithmetic_v<Y> &&
                std::is_arithmetic_v<W> && std::is_arithmetic_v<H>
            >
        >
        Rect_(X x_in, Y y_in, W w_in, H h_in)
            : x(static_cast<T>(x_in))
            , y(static_cast<T>(y_in))
            , w(static_cast<T>(w_in))
            , h(static_cast<T>(h_in))
        {}

        // Converting constructor from another Rect type
        template<typename U>
        explicit Rect_(const Rect_<U>& other)
            : x(static_cast<T>(other.x))
            , y(static_cast<T>(other.y))
            , w(static_cast<T>(other.w))
            , h(static_cast<T>(other.h))
        {}
        
        template <class Archive>
        void serialize(Archive& archive) {
            archive (
                cereal::make_nvp("x", x), cereal::make_nvp("y", y),
                cereal::make_nvp("w", w), cereal::make_nvp("h", h)
            );
        }

        // In Salix/math/Rect.h, inside the struct
        inline bool operator==(const Rect_& other) const {
            return x == other.x && y == other.y && w == other.w && h == other.h;
        }
        inline bool operator!=(const Rect_& other) const {
            return !(*this == other);
        }
    };

    // --- OSTREAM TEMPLATED HELPER FUNCTION ---
    // This teaches std::ostream how to handle any kind of Rect_<T>
    template<typename T>
    inline std::ostream& operator<<(std::ostream& os, const Rect_<T>& r) {
        os << "{ x: " << r.x << ", y: " << r.y << ", w: " << r.w << ", h: " << r.h << " }";
        return os;
    }
    // --- Aliases ---
    using Rect  = Rect_<int>;
    using RectI = Rect_<int>;
    using RectF = Rect_<float>;
    using RectD = Rect_<double>;

} // namespace Salix