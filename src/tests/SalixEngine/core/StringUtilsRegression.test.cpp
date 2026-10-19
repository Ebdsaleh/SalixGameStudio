// Tests for string case transforms, separator policies and token classification.
#include <doctest.h>
#include <Salix/core/StringUtils.h>

TEST_SUITE("Salix::core::StringUtilsRegression") {
    TEST_CASE("string utils uppercase transformation leaves digits and punctuation untouched") {
        CHECK(Salix::StringUtils::to_upper("aB9!? z") == "AB9!? Z");
        CHECK(Salix::StringUtils::to_upper("") == "");
    }

    TEST_CASE("string utils uppercase conversion expands valid snake separators when requested") {
        CHECK(Salix::StringUtils::to_upper("world_tree_2d", true) == "WORLD TREE 2D");
        CHECK(Salix::StringUtils::to_upper("world_tree_2d") == "WORLD_TREE_2D");
    }

    TEST_CASE("string utils lowercase transformation preserves punctuation and digits") {
        CHECK(Salix::StringUtils::to_lower("A1B-C_4") == "a1b-c_4");
        CHECK(Salix::StringUtils::to_lower("") == "");
    }

    TEST_CASE("string utils lowercase conversion expands valid snake separators only on opt-in") {
        CHECK(Salix::StringUtils::to_lower("MAIN_CAMERA_2D", true) == "main camera 2d");
        CHECK(Salix::StringUtils::to_lower("MAIN_CAMERA_2D") == "main_camera_2d");
    }

    TEST_CASE("string utils title casing resets at whitespace but preserves whitespace characters") {
        CHECK(Salix::StringUtils::to_title_case("hELLO\tWORLD  aGAIN") == "Hello\tWorld  Again");
        CHECK(Salix::StringUtils::to_title_case("") == "");
    }

    TEST_CASE("string utils title casing snake names respects requested separator conversion") {
        CHECK(Salix::StringUtils::to_title_case("HELLO_WORLD", true) == "Hello World");
        CHECK(Salix::StringUtils::to_title_case("HELLO_WORLD") == "Hello_world");
    }

    TEST_CASE("string utils capitalize changes only the first character to uppercase after lowering") {
        CHECK(Salix::StringUtils::capitalize("hELLO WORLD") == "Hello world");
        CHECK(Salix::StringUtils::capitalize("CAMERA_VIEW", true) == "Camera view");
        CHECK(Salix::StringUtils::capitalize("") == "");
    }

    TEST_CASE("string utils snake identification rejects malformed separators and whitespace") {
        CHECK(Salix::StringUtils::is_snake_case("camera_2d"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("_camera"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("camera_"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("a__b"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("camera view"));
    }

    TEST_CASE("string utils snake segment discovery and conversion preserve invalid underscores") {
        CHECK(Salix::StringUtils::contains_snake_case("prefix a_b suffix"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("_a_"));
        CHECK(Salix::StringUtils::convert_from_snake_case("__a_b__") == "__a b__");
        CHECK(Salix::StringUtils::convert_from_snake_case("a_b_c") == "a b c");
    }

    TEST_CASE("string utils Pascal name recognition and digit transitions produce stable labels") {
        CHECK(Salix::StringUtils::is_pascal_case("Sprite2D"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("sprite2D"));
        CHECK(Salix::StringUtils::contains_pascal_case("worldTree"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("WORLD"));
        CHECK(Salix::StringUtils::convert_from_pascal_case("Version2Alpha") == "Version 2 Alpha");
        CHECK(Salix::StringUtils::convert_from_pascal_case("Sprite2D") == "Sprite 2D");
    }
}
