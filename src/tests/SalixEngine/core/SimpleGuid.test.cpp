// Tests/core/SimpleGuid.test.cpp
#include <doctest.h>
#include <Salix/core/SimpleGuid.h> // The class we are testing

TEST_CASE("SimpleGuid: Generation and Uniqueness") {
    Salix::SimpleGuid guid1 = Salix::SimpleGuid::generate();
    Salix::SimpleGuid guid2 = Salix::SimpleGuid::generate();

    CHECK(guid1.is_valid() == true);
    CHECK(guid2.is_valid() == true);
    CHECK(guid1 != guid2);
}