#include <doctest.h>
#include <Salix/core/StringUtils.h>
#include <string>

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
