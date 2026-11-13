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

TEST_SUITE("Salix::core::StringUtilsBatch3147") {
    TEST_CASE("batch3147 uppercase desnake replaces valid interior separator but preserves edge separators") {
        CHECK(Salix::StringUtils::to_upper("_alpha_beta_", true) == "_ALPHA BETA_");
    }

    TEST_CASE("batch3147 lowercase desnake preserves doubled separator while replacing later valid separator") {
        CHECK(Salix::StringUtils::to_lower("A__B_C", true) == "a__b c");
    }

    TEST_CASE("batch3147 title case recognizes tab and newline as word boundaries") {
        CHECK(Salix::StringUtils::to_title_case("hELLO\twORLD\naGAIN") == "Hello\tWorld\nAgain");
    }

    TEST_CASE("batch3147 capitalize with desnake lowercases complete suffix including numeric token") {
        CHECK(Salix::StringUtils::capitalize("MIXED_CASE_123", true) == "Mixed case 123");
    }

    TEST_CASE("batch3147 snake validation accepts numeric segments separated by underscores") {
        CHECK(Salix::StringUtils::is_snake_case("1_2_3"));
        CHECK(Salix::StringUtils::is_snake_case("A1_2B_33"));
    }

    TEST_CASE("batch3147 snake validation requires at least one underscore") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("ABC123"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("123"));
    }

    TEST_CASE("batch3147 snake substring detection can find a valid pair inside punctuation") {
        CHECK(Salix::StringUtils::contains_snake_case("!bad?a_b;tail"));
    }

    TEST_CASE("batch3147 snake conversion preserves doubled separator and converts independent valid separator") {
        CHECK(Salix::StringUtils::convert_from_snake_case("a__b_c") == "a__b c");
    }

    TEST_CASE("batch3147 Pascal substring detection requires lowercase directly before uppercase") {
        CHECK(Salix::StringUtils::contains_pascal_case("aB1"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("a1B"));
    }

    TEST_CASE("batch3147 Pascal conversion separates acronym digit and following capitalized word boundaries") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("HTTP2Server") == "HTTP 2 Server");
    }
}
TEST_SUITE("Salix::core::StringUtilsBatch3547") {
    TEST_CASE("batch3547 uppercase without desnake preserves underscores while changing letters") {
        CHECK(Salix::StringUtils::to_upper("alpha_beta_2d", false) == "ALPHA_BETA_2D");
    }

    TEST_CASE("batch3547 lowercase without desnake preserves underscores while changing letters") {
        CHECK(Salix::StringUtils::to_lower("ALPHA_BETA_2D", false) == "alpha_beta_2d");
    }

    TEST_CASE("batch3547 title case treats carriage return and form feed as word boundaries") {
        CHECK(Salix::StringUtils::to_title_case("hELLO\rwORLD\fNEXT") == "Hello\rWorld\fNext");
    }

    TEST_CASE("batch3547 capitalize accepts leading digit and lowercases following alphabetic suffix") {
        CHECK(Salix::StringUtils::capitalize("1HELLO_WORLD", false) == "1hello_world");
    }

    TEST_CASE("batch3547 snake validation rejects consecutive underscores") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha__beta"));
    }

    TEST_CASE("batch3547 snake validation rejects hyphenated text even when underscore is present") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta-gamma"));
    }

    TEST_CASE("batch3547 snake substring detection can ignore invalid edge underscores around valid interior pair") {
        CHECK(Salix::StringUtils::contains_snake_case("_alpha_beta_"));
    }

    TEST_CASE("batch3547 snake conversion preserves edge underscores but converts valid interior separator") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_alpha_beta_") == "_alpha beta_");
    }

    TEST_CASE("batch3547 Pascal validation accepts single uppercase letter and uppercase alphanumeric token") {
        CHECK(Salix::StringUtils::is_pascal_case("A"));
        CHECK(Salix::StringUtils::is_pascal_case("A1B2"));
    }

    TEST_CASE("batch3547 Pascal conversion keeps acronym run intact while splitting later lowercase transition and digit") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("XMLHttp2Parser") == "XMLHttp 2 Parser");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch3947") {
    TEST_CASE("batch3947 uppercase empty input remains empty with desnake enabled") {
        CHECK(Salix::StringUtils::to_upper("", true).empty());
    }

    TEST_CASE("batch3947 lowercase empty input remains empty with desnake enabled") {
        CHECK(Salix::StringUtils::to_lower("", true).empty());
    }

    TEST_CASE("batch3947 title case treats vertical tab as a word boundary") {
        CHECK(Salix::StringUtils::to_title_case("fIRST\vSECOND") == "First\vSecond");
    }

    TEST_CASE("batch3947 capitalize does not skip leading whitespace before choosing first character") {
        CHECK(Salix::StringUtils::capitalize(" HELLO WORLD") == " hello world");
    }

    TEST_CASE("batch3947 minimal numeric snake token is accepted") {
        CHECK(Salix::StringUtils::is_snake_case("0_0"));
    }

    TEST_CASE("batch3947 minimal three character snake segment is detected") {
        CHECK(Salix::StringUtils::contains_snake_case("a_b"));
    }

    TEST_CASE("batch3947 incomplete snake separators are never detected as valid segments") {
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("a_"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("_b"));
    }

    TEST_CASE("batch3947 snake conversion replaces each independent valid separator in a chain") {
        CHECK(Salix::StringUtils::convert_from_snake_case("a_b_c_d") == "a b c d");
    }

    TEST_CASE("batch3947 Pascal validation accepts mixed case alphanumeric token beginning uppercase") {
        CHECK(Salix::StringUtils::is_pascal_case("Abc123Def"));
    }

    TEST_CASE("batch3947 Pascal conversion distinguishes numeric suffix from following capitalized word") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("A2Alpha") == "A 2 Alpha");
        CHECK(Salix::StringUtils::convert_from_pascal_case("A2D") == "A 2D");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch4247") {
    TEST_CASE("batch4247 uppercase conversion can desnake before case conversion") {
        CHECK(Salix::StringUtils::to_upper("hello_world_4247", true) == "HELLO WORLD 4247");
    }

    TEST_CASE("batch4247 lowercase conversion can desnake before case conversion") {
        CHECK(Salix::StringUtils::to_lower("HELLO_WORLD_4247", true) == "hello world 4247");
    }

    TEST_CASE("batch4247 title case normalizes mixed case after valid snake conversion") {
        CHECK(Salix::StringUtils::to_title_case("hELLO_wORLD_4247", true) == "Hello World 4247");
    }

    TEST_CASE("batch4247 capitalize lowers remainder after snake conversion") {
        CHECK(Salix::StringUtils::capitalize("hELLO_wORLD_4247", true) == "Hello world 4247");
    }

    TEST_CASE("batch4247 snake validation accepts alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("player2_camera4247"));
        CHECK(Salix::StringUtils::contains_snake_case("player2_camera4247"));
    }

    TEST_CASE("batch4247 snake validation rejects doubled underscore") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("player__camera"));
    }

    TEST_CASE("batch4247 contains snake case can detect valid segment inside otherwise punctuated text") {
        CHECK(Salix::StringUtils::contains_snake_case("prefix-player_camera-suffix"));
    }

    TEST_CASE("batch4247 snake conversion replaces only underscores with alphanumeric neighbors") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_a_b_") == "_a b_");
    }

    TEST_CASE("batch4247 Pascal validation accepts uppercase acronym with digits") {
        CHECK(Salix::StringUtils::is_pascal_case("HTTP2D"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("HTTP2D"));
    }

    TEST_CASE("batch4247 Pascal conversion separates digit to title word transition") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Version2Alpha") == "Version 2 Alpha");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch4647") {
    TEST_CASE("batch4647 uppercase conversion preserves digits punctuation and spacing") {
        CHECK(Salix::StringUtils::to_upper("a1-b2 c3!", false) == "A1-B2 C3!");
    }

    TEST_CASE("batch4647 lowercase conversion preserves digits punctuation and spacing") {
        CHECK(Salix::StringUtils::to_lower("A1-B2 C3!", false) == "a1-b2 c3!");
    }

    TEST_CASE("batch4647 title case preserves repeated spaces while normalizing each word") {
        CHECK(Salix::StringUtils::to_title_case("hELLO   wORLD") == "Hello   World");
    }

    TEST_CASE("batch4647 title case without desnake treats underscore as an ordinary nonspace character") {
        CHECK(Salix::StringUtils::to_title_case("hELLO_WORLD", false) == "Hello_world");
    }

    TEST_CASE("batch4647 capitalize with desnake preserves edge underscores while lowering remainder") {
        CHECK(Salix::StringUtils::capitalize("_HELLO_WORLD_", true) == "_hello world_");
    }

    TEST_CASE("batch4647 snake validation accepts several mixed alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("A1_b2_C3_4647"));
    }

    TEST_CASE("batch4647 snake validation rejects period even when valid underscore segment exists") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta.txt"));
    }

    TEST_CASE("batch4647 snake substring detection finds valid segment surrounded by parentheses") {
        CHECK(Salix::StringUtils::contains_snake_case("(alpha_beta)"));
    }

    TEST_CASE("batch4647 snake conversion leaves underscores whose neighbors are punctuation untouched") {
        CHECK(Salix::StringUtils::convert_from_snake_case("a_!_b") == "a_!_b");
    }

    TEST_CASE("batch4647 Pascal conversion keeps multi digit token together before title word") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("Version42Alpha") == "Version 42 Alpha");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch5047") {
    TEST_CASE("batch5047 title case treats tab and newline as word boundaries") {
        CHECK(Salix::StringUtils::to_title_case("aLPHA\tbETA\ngAMMA") == "Alpha\tBeta\nGamma");
    }

    TEST_CASE("batch5047 capitalize preserves leading whitespace while lowercasing remaining letters") {
        CHECK(Salix::StringUtils::capitalize(" HELLO WORLD") == " hello world");
    }

    TEST_CASE("batch5047 snake validation accepts digit-only segments around separators") {
        CHECK(Salix::StringUtils::is_snake_case("12_34_56"));
    }

    TEST_CASE("batch5047 snake validation rejects hyphen even when another valid separator exists") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta-gamma"));
    }

    TEST_CASE("batch5047 snake substring detection finds inner segment surrounded by underscores") {
        CHECK(Salix::StringUtils::contains_snake_case("__a_b__"));
    }

    TEST_CASE("batch5047 snake conversion preserves doubled separators without alphanumeric neighbors") {
        CHECK(Salix::StringUtils::convert_from_snake_case("alpha__beta") == "alpha__beta");
    }

    TEST_CASE("batch5047 single uppercase letter satisfies Pascal validation") {
        CHECK(Salix::StringUtils::is_pascal_case("A"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("A"));
    }

    TEST_CASE("batch5047 leading digit prevents Pascal validation") {
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("1Alpha"));
    }

    TEST_CASE("batch5047 Pascal substring detection requires lowercase to uppercase transition") {
        CHECK(Salix::StringUtils::contains_pascal_case("aB"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("AB"));
    }

    TEST_CASE("batch5047 Pascal conversion retains acronym run and separates following numeric token") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("HTTPServer2D") == "HTTPServer 2D");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch5447") {
    TEST_CASE("batch5447 upper conversion with desnake replaces valid underscores before casing") {
        CHECK(Salix::StringUtils::to_upper("alpha_beta_5447", true) == "ALPHA BETA 5447");
    }

    TEST_CASE("batch5447 lower conversion with desnake replaces valid underscores before casing") {
        CHECK(Salix::StringUtils::to_lower("ALPHA_BETA_5447", true) == "alpha beta 5447");
    }

    TEST_CASE("batch5447 title case preserves leading trailing and repeated whitespace") {
        CHECK(Salix::StringUtils::to_title_case("  hELLO   wORLD  ") == "  Hello   World  ");
    }

    TEST_CASE("batch5447 capitalize empty input remains empty") {
        CHECK(Salix::StringUtils::capitalize("").empty());
        CHECK(Salix::StringUtils::capitalize("", true).empty());
    }

    TEST_CASE("batch5447 snake validation rejects leading and trailing underscores") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("_alpha_beta"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta_"));
    }

    TEST_CASE("batch5447 snake validation accepts digit adjacent separator segments") {
        CHECK(Salix::StringUtils::is_snake_case("a1_2b_5447"));
    }

    TEST_CASE("batch5447 contains snake case recognizes one valid inner separator") {
        CHECK(Salix::StringUtils::contains_snake_case("prefix a_b suffix"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("prefix a__b suffix"));
    }

    TEST_CASE("batch5447 snake conversion replaces only valid alphanumeric separators") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_a_b__c_d_") == "_a b__c d_");
    }

    TEST_CASE("batch5447 Pascal validation rejects embedded punctuation but accepts digits") {
        CHECK(Salix::StringUtils::is_pascal_case("Alpha5447Beta"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("Alpha-5447"));
    }

    TEST_CASE("batch5447 Pascal conversion separates lower uppercase and alpha digit transitions") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("BatchValue5447Alpha") ==
              "Batch Value 5447 Alpha");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch5847") {
    TEST_CASE("batch5847 upper conversion preserves punctuation digits and spacing") {
        CHECK(Salix::StringUtils::to_upper("a5-b8 c4!", false) == "A5-B8 C4!");
    }

    TEST_CASE("batch5847 lower conversion with desnake replaces valid separators first") {
        CHECK(Salix::StringUtils::to_lower("HELLO_WORLD_5847", true) == "hello world 5847");
    }

    TEST_CASE("batch5847 title case recognizes tab newline and repeated spaces as boundaries") {
        CHECK(Salix::StringUtils::to_title_case("hELLO\twORLD\nbATCH   fIVE") ==
              "Hello\tWorld\nBatch   Five");
    }

    TEST_CASE("batch5847 capitalize lowers remainder without skipping leading whitespace") {
        CHECK(Salix::StringUtils::capitalize(" HELLO_B5847", false) == " hello_b5847");
    }

    TEST_CASE("batch5847 snake validation accepts mixed alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("alpha5_beta8_5847"));
        CHECK(Salix::StringUtils::contains_snake_case("alpha5_beta8_5847"));
    }

    TEST_CASE("batch5847 snake validation rejects doubled and edge underscores") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("_alpha_beta"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha__beta"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta_"));
    }

    TEST_CASE("batch5847 snake substring detection ignores invalid doubled separator") {
        CHECK(Salix::StringUtils::contains_snake_case("prefix a_b suffix"));
        CHECK_FALSE(Salix::StringUtils::contains_snake_case("prefix a__b suffix"));
    }

    TEST_CASE("batch5847 snake conversion replaces only separators with alphanumeric neighbors") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_a_b__c_d_") == "_a b__c d_");
    }

    TEST_CASE("batch5847 Pascal validation accepts digits but rejects punctuation") {
        CHECK(Salix::StringUtils::is_pascal_case("Alpha5847Beta"));
        CHECK_FALSE(Salix::StringUtils::is_pascal_case("Alpha-5847"));
    }

    TEST_CASE("batch5847 Pascal conversion separates lower upper and numeric word transitions") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("BatchValue5847Alpha") ==
              "Batch Value 5847 Alpha");
    }
}

TEST_SUITE("Salix::core::StringUtilsBatch6247") {
    TEST_CASE("batch6247 upper conversion keeps empty input empty") {
        CHECK(Salix::StringUtils::to_upper("", false).empty());
        CHECK(Salix::StringUtils::to_upper("", true).empty());
    }

    TEST_CASE("batch6247 lower conversion keeps punctuation and digits unchanged") {
        CHECK(Salix::StringUtils::to_lower("A6-B2_C4!", false) == "a6-b2_c4!");
    }

    TEST_CASE("batch6247 title case with desnake converts valid separators into word boundaries") {
        CHECK(Salix::StringUtils::to_title_case("aLPHA_bETA_6247", true) ==
              "Alpha Beta 6247");
    }

    TEST_CASE("batch6247 capitalize with desnake lowercases later words after separator removal") {
        CHECK(Salix::StringUtils::capitalize("aLPHA_bETA", true) == "Alpha beta");
    }

    TEST_CASE("batch6247 snake validation accepts uppercase alphanumeric segments") {
        CHECK(Salix::StringUtils::is_snake_case("ALPHA6_BETA2_47"));
    }

    TEST_CASE("batch6247 snake validation rejects embedded whitespace") {
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta gamma"));
        CHECK_FALSE(Salix::StringUtils::is_snake_case("alpha_beta\ngamma"));
    }

    TEST_CASE("batch6247 snake substring detection recognizes digit neighbors") {
        CHECK(Salix::StringUtils::contains_snake_case("prefix 6_2 suffix"));
        CHECK(Salix::StringUtils::contains_snake_case("x A_7 y"));
    }

    TEST_CASE("batch6247 snake conversion keeps edge separators while replacing valid inner ones") {
        CHECK(Salix::StringUtils::convert_from_snake_case("_A_6_B_2_") ==
              "_A 6 B 2_");
    }

    TEST_CASE("batch6247 Pascal validation accepts all uppercase alphanumeric token") {
        CHECK(Salix::StringUtils::is_pascal_case("ABC6247"));
        CHECK_FALSE(Salix::StringUtils::contains_pascal_case("ABC6247"));
    }

    TEST_CASE("batch6247 Pascal conversion separates alpha digit and digit word transitions") {
        CHECK(Salix::StringUtils::convert_from_pascal_case("A1Beta2Gamma") ==
              "A 1 Beta 2 Gamma");
    }
}
