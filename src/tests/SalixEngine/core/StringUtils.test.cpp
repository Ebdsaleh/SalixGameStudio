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

// Boundary behavior pinned against the implementation and public API.
TEST_SUITE("Salix::Batch747::StringUtilsBatch747") {
    TEST_CASE("strings empty input is stable through every transformation and classifier") {
        using S=Salix::StringUtils; CHECK(S::to_upper("").empty());CHECK(S::to_lower("").empty());CHECK(S::to_title_case("").empty());CHECK(S::capitalize("").empty());CHECK(S::convert_from_snake_case("").empty());CHECK(S::convert_from_pascal_case("").empty());CHECK_FALSE(S::is_snake_case(""));CHECK_FALSE(S::is_pascal_case(""));
    }

    TEST_CASE("strings case conversion preserves an embedded NUL byte and suffix") {
        std::string source("ab\0Cd",5);const auto upper=Salix::StringUtils::to_upper(source);const auto lower=Salix::StringUtils::to_lower(source);REQUIRE(upper.size()==5);REQUIRE(lower.size()==5);CHECK(upper==std::string("AB\0CD",5));CHECK(lower==std::string("ab\0cd",5));
    }

    TEST_CASE("strings title case resets on repeated mixed whitespace without discarding separators") {
        CHECK(Salix::StringUtils::to_title_case("hELLo  \twORLD\nagain")=="Hello  \tWorld\nAgain");
    }

    TEST_CASE("strings capitalization lowercases subsequent words without introducing title-case") {
        CHECK(Salix::StringUtils::capitalize("sALIX ENGINE TEST")=="Salix engine test");
    }

    TEST_CASE("strings numeric snake segments are replaced only between alphanumeric neighbors") {
        using S=Salix::StringUtils;CHECK(S::is_snake_case("file_12_version_3"));CHECK(S::convert_from_snake_case("file_12_version_3")=="file 12 version 3");CHECK_FALSE(S::is_snake_case("file__12"));
    }

    TEST_CASE("strings malformed snake separators remain distinguishable from valid internal segments") {
        using S=Salix::StringUtils;CHECK(S::contains_snake_case("__good_name__"));CHECK_FALSE(S::is_snake_case("__good_name__"));CHECK(S::convert_from_snake_case("__good_name__")=="__good name__");
    }

    TEST_CASE("strings titlecase snake opt-in changes only valid separators") {
        CHECK(Salix::StringUtils::to_title_case("HELLO_WORLD__TEST",true)=="Hello World__test");CHECK(Salix::StringUtils::to_title_case("HELLO_WORLD__TEST",false)=="Hello_world__test");
    }

    TEST_CASE("strings Pascal conversion preserves an uppercase acronym without inventing word boundaries") {
        using S=Salix::StringUtils;CHECK(S::is_pascal_case("HTTPServer"));CHECK(S::convert_from_pascal_case("HTTPServer")=="HTTPServer");CHECK_FALSE(S::contains_pascal_case("HTTPServer"));
    }

    TEST_CASE("strings Pascal conversion separates a digit followed by a genuine capitalized word") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Level12Alpha")=="Level 12 Alpha");CHECK(Salix::StringUtils::convert_from_pascal_case("Sprite2D")=="Sprite 2D");
    }

    TEST_CASE("strings Pascal validation rejects punctuation but accepts a single leading capital") {
        using S=Salix::StringUtils;CHECK(S::is_pascal_case("A"));CHECK(S::is_pascal_case("A123"));CHECK_FALSE(S::is_pascal_case("A_Name"));CHECK_FALSE(S::is_pascal_case("A-Name"));CHECK_FALSE(S::is_pascal_case("1Name"));
    }
}

TEST_SUITE("Salix::core::StringUtilsClosure1747") {
    TEST_CASE("batch1747 string utils snake validation accepts digit bearing alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("A1_B2"));
        CHECK(Salix::StringUtils::is_snake_case("version2_alpha3"));
    }

    TEST_CASE("batch1747 string utils full snake validation rejects double underscores while substring detection still finds valid later segment") {
        const std::string value = "bad__name_ok";
        CHECK_FALSE(Salix::StringUtils::is_snake_case(value));
        CHECK(Salix::StringUtils::contains_snake_case(value));
    }

    TEST_CASE("batch1747 string utils snake conversion replaces only underscores with valid alphanumeric neighbors") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_a_b__c_d_") == "_a b__c d_");
    }

    TEST_CASE("batch1747 string utils Pascal validation accepts a single uppercase letter followed by digits") {
        CHECK(Salix::StringUtils::is_pascal_case("A1"));
        CHECK(Salix::StringUtils::is_pascal_case("X2026"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("x2026"));
    }

    TEST_CASE("batch1747 string utils Pascal substring detection finds lower to upper transition around digits") {
        CHECK(Salix::StringUtils::contains_pascal_case("abcDef2"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("ABC2DEF"));
    }

    TEST_CASE("batch1747 string utils Pascal conversion preserves leading uppercase acronym as one token") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("XMLParser") == "XMLParser");
    }

    TEST_CASE("batch1747 string utils Pascal conversion separates Version2Alpha at both semantic boundaries") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Version2Alpha") == "Version 2 Alpha");
    }

    TEST_CASE("batch1747 string utils Pascal conversion keeps Sprite2D suffix together after numeric transition") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Sprite2D") == "Sprite 2D");
    }

    TEST_CASE("batch1747 string utils title case with snake conversion preserves repeated whitespace") {
        CHECK(Salix::StringUtils::to_title_case("hello_world  AGAIN", true) == "Hello World  Again");
    }

    TEST_CASE("batch1747 string utils capitalize with snake conversion lowers all characters after first") {
        CHECK(Salix::StringUtils::capitalize("HELLO_WORLD", true) == "Hello world");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch2147") {
    TEST_CASE("batch2147 uppercase snake conversion is opt in and preserves separator choice") {
        CHECK(Salix::StringUtils::to_upper("main_camera_2d", false) == "MAIN_CAMERA_2D");
        CHECK(Salix::StringUtils::to_upper("main_camera_2d", true) == "MAIN CAMERA 2D");
    }
    TEST_CASE("batch2147 lowercase snake conversion is opt in and preserves malformed separators") {
        CHECK(Salix::StringUtils::to_lower("MAIN_CAMERA", true) == "main camera");
        CHECK(Salix::StringUtils::to_lower("MAIN__CAMERA", true) == "main__camera");
    }
    TEST_CASE("batch2147 title case recognizes every standard ASCII whitespace boundary") {
        CHECK(Salix::StringUtils::to_title_case("oNE\ttWO\nTHREE\rFOUR") == "One\tTwo\nThree\rFour");
    }
    TEST_CASE("batch2147 capitalize lowercases the complete suffix after the first character") {
        CHECK(Salix::StringUtils::capitalize("sALIX Engine TEST") == "Salix engine test");
    }
    TEST_CASE("batch2147 snake validation accepts several alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("scene2_camera3_mode4"));
        CHECK(Salix::StringUtils::is_snake_case("A1_B2_C3"));
    }
    TEST_CASE("batch2147 snake validation rejects edge and repeated separators") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("_abc_def"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("abc_def_"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("abc__def"));
    }
    TEST_CASE("batch2147 snake substring detection can succeed inside otherwise invalid text") {
        CHECK(Salix::StringUtils::contains_snake_case("!abc_def?"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("!abc_def?"));
    }
    TEST_CASE("batch2147 snake conversion replaces only separators with alphanumeric neighbors") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_a_b__c_d_") == "_a b__c d_");
    }
    TEST_CASE("batch2147 Pascal conversion separates lower upper and alpha digit transitions") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("PlayerModel2") == "Player Model 2");
    }
    TEST_CASE("batch2147 Pascal conversion separates digit to capital only for a new capitalized word") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Version2Alpha") == "Version 2 Alpha");
        CHECK(Salix::StringUtils::convert_from_pascal_case("Sprite2D") == "Sprite 2D");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch2647") {
    TEST_CASE("batch2647 title case preserves leading trailing and repeated whitespace") {
        CHECK(Salix::StringUtils::to_title_case("  hELLO   wORLD  ") == "  Hello   World  ");
    }
    TEST_CASE("batch2647 title case can desnakify before applying word casing") {
        CHECK(Salix::StringUtils::to_title_case("hELLO_wORLD_2d", true) == "Hello World 2d");
    }
    TEST_CASE("batch2647 capitalize can desnakify while lowercasing the remaining text") {
        CHECK(Salix::StringUtils::capitalize("mAIN_CAMERA_MODE", true) == "Main camera mode");
    }
    TEST_CASE("batch2647 valid snake case may mix letters and digits around separators") {
        CHECK(Salix::StringUtils::is_snake_case("camera2_mode3"));
        CHECK(Salix::StringUtils::is_snake_case("A1_b2"));
    }
    TEST_CASE("batch2647 snake case rejects punctuation even when underscore placement is valid") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("camera_mode!"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("camera-mode"));
    }
    TEST_CASE("batch2647 contains snake case rejects isolated leading trailing and doubled underscores") {
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("_abc"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("abc_"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("a__b"));
    }
    TEST_CASE("batch2647 snake conversion preserves underscores without two alphanumeric neighbors") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_a__b_") == "_a__b_");
    }
    TEST_CASE("batch2647 a single uppercase alphanumeric token satisfies current Pascal contract") {
        CHECK(Salix::StringUtils::is_pascal_case("A"));
        CHECK(Salix::StringUtils::is_pascal_case("A123"));
    }
    TEST_CASE("batch2647 Pascal validation rejects lowercase starts and nonalphanumeric content") {
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("cameraMode"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("Camera_Mode"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case(""));
    }
    TEST_CASE("batch2647 Pascal conversion separates lower uppercase and letter digit transitions") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("BoxCollider2D") == "Box Collider 2D");
        CHECK(Salix::StringUtils::contains_pascal_case("BoxCollider2D"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("HTTP2D"));
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch2747") {
    TEST_CASE("batch2747 uppercase conversion preserves digits and punctuation") {
        CHECK(Salix::StringUtils::to_upper("salix-2.0!", false)=="SALIX-2.0!");
    }
    TEST_CASE("batch2747 lowercase conversion preserves digits and punctuation") {
        CHECK(Salix::StringUtils::to_lower("SALIX-2.0!", false)=="salix-2.0!");
    }
    TEST_CASE("batch2747 title case leaves empty string empty") {
        CHECK(Salix::StringUtils::to_title_case("").empty());
    }
    TEST_CASE("batch2747 title case normalizes mixed casing across multiple spaces") {
        CHECK(Salix::StringUtils::to_title_case("hELLo   wORLD")=="Hello   World");
    }
    TEST_CASE("batch2747 capitalize handles a single character") {
        CHECK(Salix::StringUtils::capitalize("x")=="X");
    }
    TEST_CASE("batch2747 capitalize leaves empty string unchanged") {
        CHECK(Salix::StringUtils::capitalize("").empty());
    }
    TEST_CASE("batch2747 snake detection rejects whitespace-separated words") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("hello world"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("hello world"));
    }
    TEST_CASE("batch2747 snake detection accepts uppercase alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("PLAYER_2_CAMERA"));
        CHECK(Salix::StringUtils::contains_snake_case("PLAYER_2_CAMERA"));
    }
    TEST_CASE("batch2747 snake conversion leaves strings without valid separators unchanged") {
        CHECK(Salix::StringUtils::convert_from_snake_case("plain")=="plain");
        CHECK(Salix::StringUtils::convert_from_snake_case("_plain_")=="_plain_");
    }
    TEST_CASE("batch2747 Pascal conversion preserves already separated lowercase word") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("plain")=="plain");
        CHECK(Salix::StringUtils::convert_from_pascal_case("")=="");
    }
}

