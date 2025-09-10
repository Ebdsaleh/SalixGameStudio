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