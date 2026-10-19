// =================================================================================
// Filename:    src/Tests/SalixEngine/core/StringUtils.test.cpp
// Description: Contains unit tests for the StringUtils functions.
// =================================================================================

#include <doctest.h>
#include <Salix/core/StringUtils.h>
#include <string>
#include <algorithm>
#include <cctype>

TEST_SUITE("Salix::core::StringUtils") {
    TEST_CASE("to_upper correctly converts strings to uppercase") {
        // ARRANGE
        std::string input = "hello world";
        std::string expected = "HELLO WORLD";

        // ACT
        std::string result = Salix::StringUtils::to_upper(input, false);

        // ASSERT
        CHECK(result == expected);
        

        SUBCASE("with snake_case removal") {
            // ARRANGE
            input = "hello_world";
            expected = "HELLO WORLD";

            // ACT
            result = Salix::StringUtils::to_upper(input, true);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return ' ' when string is empty") {
            // ARRANGE
            input = "";
            expected = "";

            // ACT
            result = Salix::StringUtils::to_upper(input);

            // ASSERT
            CHECK(result == expected);
        }
    }
    TEST_CASE("to_lower correctly converts strings to lowercase") {
        // ARRANGE
        std::string input = "HELLO WORLD";
        std::string expected = "hello world";

        // ACT 
        std::string result  = Salix::StringUtils::to_lower(input);

        // ASSERT
        CHECK(result == expected);

        SUBCASE("with snake_case removal") {
            // ARRANGE
            input = "HELLO_WORLD";
            expected = "hello world";

            // ACT 
            result = Salix::StringUtils::to_lower(input, true);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return ' ' when string is empty") {
            // ARRANGE
            input = "";
            expected = "";

            // ACT
            result = Salix::StringUtils::to_lower(input, true);

            // ASSERT
            CHECK(result == expected);
        }
    }
    TEST_CASE("to_title_case correctly converts strings to titlecase") {
        // ARRANGE
        std::string input = "hello world";
        std::string expected = "Hello World";

        // ACT 
        std::string result  = Salix::StringUtils::to_title_case(input);

        // ASSERT
        CHECK(result == expected);

        SUBCASE("with snake_case removal") {
            // ARRANGE
             input = "hello_world";
             expected = "Hello World";

            // ACT 
            result = Salix::StringUtils::to_title_case(input, true);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return ' ' when string is empty") {
            // ARRANGE
            input = "";
            expected = "";

            // ACT
            result = Salix::StringUtils::to_title_case(input, true);

            // ASSERT
            CHECK(result == expected);
        }
    }
    TEST_CASE("capitalize correctly converts the first letter in the string to uppercase and the other to lowercase") {
        // ARRANGE
        std::string input = "hello world";
        std::string expected = "Hello world";

        // ACT 
        std::string result  = Salix::StringUtils::capitalize(input);

        // ASSERT
        CHECK(result == expected);

        SUBCASE("with snake_case removal") {
            // ARRANGE
            input = "hello_world";
            expected = "Hello world";

            // ACT 
            result = Salix::StringUtils::capitalize(input, true);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return ' ' when string is empty") {
            // ARRANGE
            input = "";
            expected = "";

            // ACT
            result = Salix::StringUtils::capitalize(input);

            // ASSERT
            CHECK(result == expected);
        }
    }
    TEST_CASE("correctly determines the string is exclusively snake_case") {
        // ARRANGE
        std::string input  = "hello_world";
        bool expected = true;

        // ACT
        bool result = Salix::StringUtils::is_snake_case(input);

        // ASSERT
        CHECK(result == expected);
        
        SUBCASE("correctly determines the string is not snake_case") {
            // ARRANGE
            input = "hello world Hell_o";
            expected = false;

            // ACT
            result = Salix::StringUtils::is_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("correctly determines the string is not snake_case, using mixed cases") {
            // ARRANGE
            input = "hello_world hello";
            expected = false;

            // ACT
            result = Salix::StringUtils::is_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("correctly determines the string is not valid snake_case, with '_' prefix") {
            // ARRANGE
            input = "_hello_world";
            expected = false;

            // ACT
            result = Salix::StringUtils::is_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("correctly determines the string is not valid snake_case, with '_' suffix") {
            // ARRANGE
            input = "hello_world_";
            expected = false;

            // ACT
            result = Salix::StringUtils::is_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("correctly determines the string is not valid snake_case, with '_' for prefix and suffix") {
            // ARRANGE
            input = "_hello_world_";
            expected = false;

            // ACT
            result = Salix::StringUtils::is_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return false when string is empty") {
            // ARRANGE
            input = "";
            bool expected_bool = false;

            // ACT
            result = Salix::StringUtils::is_snake_case(input);

            // ASSERT
            CHECK(result == expected_bool);
        }
    }
    TEST_CASE("correctly determines the string contains snake_case") {
        // ARRANGE
        std::string input = "helloworld hello_world";
        bool expected = true;

        // ACT
        bool result = Salix::StringUtils::contains_snake_case(input);

        // ASSERT
        CHECK(result == expected);

        SUBCASE("correctly determines with only snake_case word") {
            //ARRANGE
            input = "hello_world";
            expected = true;

            // ACT
            result = Salix::StringUtils::contains_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("correctly determines the string does not snake_case") {
            //ARRANGE
            input = "hello world";
            expected = false;

            // ACT
            result = Salix::StringUtils::contains_snake_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return false when string is empty") {
            // ARRANGE
            input = "";
            bool expected_bool = false;

            // ACT
            result = Salix::StringUtils::contains_snake_case(input);

            // ASSERT
            CHECK(result == expected_bool);
        }
    }
    TEST_CASE("convert_from_snake_case works correctly") {
        // ARRANGE
        std::string input = "he_llowor_ld hello_world";
        std::string expected_str = "he llowor ld hello world";
        // expecting false, because will be using 'contains_snake_case' function to determine the result.
        // a result of false will determine success.

        // ACT
        std::string result_str = Salix::StringUtils::convert_from_snake_case(input);
        // ASSERT
        CHECK(result_str == expected_str);

        SUBCASE("will return ' ' when string is empty") {
            // ARRANGE
            input = "";
            expected_str = "";

            // ACT
            result_str = Salix::StringUtils::convert_from_snake_case(input);
    
            // ASSERT
            CHECK(result_str == expected_str);
        }
    }
    TEST_CASE("correctly determines if a string is exclusively PascalCase") {
        // ARRANGE
        std::string input = "HelloWorld";
                
        bool expected = true;

        // ACT
       
        bool result = Salix::StringUtils::is_pascal_case(input);
        // ASSERT
        CHECK(result == expected);
        SUBCASE ("correctly determines that at string is not exclusively PascalCase") {
            input = "HelloWorld Hello";
            expected = false;

            // ACT
            result = Salix::StringUtils::is_pascal_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return false when string is empty") {
            // ARRANGE
            input = "";
            bool expected_bool = false;

            // ACT
            result = Salix::StringUtils::is_pascal_case(input);

            // ASSERT
            CHECK(result == expected_bool);
        }
    }
    TEST_CASE("correctly determines that a string contains PascalCase") {
        // ARRANGE
        std::string input = "HelloWorld Hello";
        bool expected = true;

        // ACT
        bool result = Salix::StringUtils::contains_pascal_case(input);

        // ASSERT
        CHECK(result == expected);

        SUBCASE("correctly determines that a string does not contain PascalCase") {
            // ARRANGE
            input = "Hello_World Hello";
            expected = false;

            // ACT
            result = Salix::StringUtils::contains_pascal_case(input);

            // ASSERT
            CHECK(result == expected);
        }
        SUBCASE("will return false when string is empty") {
            // ARRANGE
            input = "";
            bool expected_bool = false;

            // ACT
            result = Salix::StringUtils::contains_pascal_case(input);

            // ASSERT
            CHECK(result == expected_bool);
        }
        SUBCASE("will return false when string length is less than 2 charcters.") {
            // ARRANGE
            input = "H";
            bool expected_bool = false;

            // ACT
            result = Salix::StringUtils::contains_pascal_case(input);

            // ASSERT
            CHECK(result == expected_bool);
        }
    
    }
    TEST_CASE("correctly converts a string from PascalCase, seperated words") {
        // ARRANGE
        std::string input = "HelloWorld";
        std::string expected_str = "Hello World";
        
        // ACT
        std::string result_str = Salix::StringUtils::convert_from_pascal_case(input);
        // ASSERT
        CHECK(result_str == expected_str);
        SUBCASE("will return ' ' when string is empty") {
            // ARRANGE
            input = "";
            expected_str = "";
            
            // ACT
            result_str = Salix::StringUtils::convert_from_pascal_case(input);
            // ASSERT
            CHECK(result_str == expected_str);
        }
        SUBCASE("correctly handles when an uppercase letter follows a digit e.g '2D'") {
            // ARRANGE
            input = "Sprite2D";
            expected_str = "Sprite 2D";
            
            // ACT
            result_str = Salix::StringUtils::convert_from_pascal_case(input);
           

            // ASSERT: Sprite2D will be converted to Sprite 2D
            CHECK(result_str == expected_str);
        }
        SUBCASE("correctly handles the transition of digit to word") {
            // ARRANGE
            input = "Version2Alpha";
            expected_str = "Version 2 Alpha";
            
            // ACT
            result_str = Salix::StringUtils::convert_from_pascal_case(input);
            

            // ASSERT: Version2Alpha will be converted to Version 2 Alpha
            CHECK(result_str == expected_str);
        }
    }
    
}

// Additional cases preserved from the separate regression suite.
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

// Character-boundary and identifier-normalization contracts for Editor labels.
TEST_SUITE("Salix::core::StringUtilsAdditional20261019") {
    TEST_CASE("strings mixed-case identifiers retain digits and punctuation during ASCII case conversion") {
        const std::string input = "Sprite2D-v3!";
        CHECK(Salix::StringUtils::to_upper(input) == "SPRITE2D-V3!");
        CHECK(Salix::StringUtils::to_lower(input) == "sprite2d-v3!");
        CHECK(input == "Sprite2D-v3!");
    }

    TEST_CASE("strings upper and lower conversion are idempotent on multiline ASCII") {
        const std::string original = "Main Camera\nPLAYER_42\tUnchanged-7";
        const auto upper = Salix::StringUtils::to_upper(original);
        const auto lower = Salix::StringUtils::to_lower(original);
        CHECK(Salix::StringUtils::to_upper(upper) == upper);
        CHECK(Salix::StringUtils::to_lower(lower) == lower);
    }

    TEST_CASE("strings title case treats tabs and newlines as boundaries while keeping digits") {
        CHECK(Salix::StringUtils::to_title_case("hELLO\tWoRLD\n3D ASSETS") == "Hello\tWorld\n3d Assets");
        CHECK(Salix::StringUtils::to_title_case("a  B") == "A  B");
    }

    TEST_CASE("strings capitalizing does not start new words after spaces") {
        CHECK(Salix::StringUtils::capitalize("eDITOR CAMERA MODE") == "Editor camera mode");
        CHECK(Salix::StringUtils::capitalize("mAIN\tPORTAL") == "Main\tportal");
    }

    TEST_CASE("strings snake converter respects leading trailing and adjacent underscores") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_root_") == "_root_");
        CHECK(Salix::StringUtils::convert_from_snake_case("a__b") == "a__b");
        CHECK(Salix::StringUtils::convert_from_snake_case("a_b__c_d") == "a b__c d");
    }

    TEST_CASE("strings snake classifier accepts numeric segments but rejects dots and dashes") {
        CHECK(Salix::StringUtils::is_snake_case("lod_0"));
        CHECK(Salix::StringUtils::is_snake_case("camera_2d"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("camera-2d"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("scene.json"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("camera__2d"));
    }

    TEST_CASE("strings snake substring detection differs from complete identifier validation") {
        CHECK(Salix::StringUtils::contains_snake_case("x_camera_mode!"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("x_camera_mode!"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("_x_"));
        CHECK(Salix::StringUtils::contains_snake_case("a_3"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case(""));
    }

    TEST_CASE("strings Pascal classifier checks capitalization without requiring a word boundary") {
        CHECK(Salix::StringUtils::is_pascal_case("A"));
        CHECK(Salix::StringUtils::is_pascal_case("JSON2"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("json2"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("Main_Camera"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("Main Camera"));
    }

    TEST_CASE("strings Pascal splitting handles a word following a numeric suffix") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Camera2Target") == "Camera 2 Target");
        CHECK(Salix::StringUtils::convert_from_pascal_case("Sprite2D") == "Sprite 2D");
        CHECK(Salix::StringUtils::convert_from_pascal_case("PlayerModel") == "Player Model");
    }

    TEST_CASE("strings snake-aware case transformations preserve malformed separators") {
        CHECK(Salix::StringUtils::to_upper("_camera_mode_", true) == "_CAMERA MODE_");
        CHECK(Salix::StringUtils::to_lower("MAIN__CAMERA", true) == "main__camera");
        CHECK(Salix::StringUtils::to_title_case("main_camera", true) == "Main Camera");
        CHECK(Salix::StringUtils::capitalize("MAIN_CAMERA", true) == "Main camera");
    }
}
