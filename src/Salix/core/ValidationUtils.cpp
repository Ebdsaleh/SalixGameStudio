// Salix/core/ValidationUtils.cpp
#include <Salix/core/ValidationUtils.h>
#include <type_traits>
#include <string>
#include <cmath>


namespace Salix::ValidationUtils {

    bool is_valid_string(const std::string& test_string, bool allow_empty, bool allow_whitespace) {
        bool is_valid = false;
        if (!allow_empty) {
            if(test_string.empty()) return is_valid;
        }
        // allow_empty is true
        else if (!allow_whitespace){
            if (test_string.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
                return is_valid;
            }
        }
        is_valid = true;
        return is_valid;
    }

    bool is_valid_c_str(const char* test_c_str) {
        bool is_valid = (test_c_str != nullptr);
        return is_valid;
    }

    // --- TODO SOON ---
    // Signed ints
    bool is_valid_int(int test_int) {

        return false;
    }

    // Unsigned ints
    bool is_valid_unsigned_int(unsigned int test_unsigned_int) {

        return false;
    }

    bool is_valid_uint8_t(uint8_t test_uint8_t) {

        return false;
    }
    bool is_valid_uint16_t(uint16_t test_uint16_t) {

        return false;
    }
    bool is_valid_uint32_t(uint32_t test_uint32_t) {

        return false;
    }
    bool is_valid_uint64_t(uint64_t test_uint64_t) {

        return false;
    }

    // --- END TODO SOON ---


    // A "valid" float is one that is not "Not a Number" (NaN) or infinity.
    bool is_valid_float(float test_float) {
        bool is_valid = !std::isnan(test_float) && !std::isinf(test_float);
        return is_valid;
    }

    bool is_valid_double(double test_double) {
        bool is_valid =  !std::isnan(test_double) && !std::isinf(test_double);
        return is_valid;
    }

}