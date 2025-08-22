// Salix/math/Rect.cpp

#include <Salix/math/Rect.h>

namespace Salix {
// This explicitly tells the compiler to generate the full class code 
// for these specific types and export them from the SalixEngine.dll.
template struct SALIX_API Salix::Rect_<int>;
template struct SALIX_API Salix::Rect_<float>;
template struct SALIX_API Salix::Rect_<double>;

} // namespace Salix