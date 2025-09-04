// Tests/math/Vector3.test.cpp
#include <doctest.h>
#include <Salix/math/Vector3.h>

TEST_CASE("Vector3 Addition") {
     // ARRANGE: Create two vectors we want to add.
    Salix::Vector3 a(1.0f, 2.0f, 3.0f);
    Salix::Vector3 b(4.0f, 5.0f, 6.0f);

    // ACT: Perform the addition.
    Salix::Vector3 result = a + b;

    // ASSERT: Check if each component of the result is correct.
    CHECK(result.x == 5.0f);
    CHECK(result.y == 7.0f);
    CHECK(result.z == 9.0f);
}

TEST_CASE("Vector3: Normalization") {
    // ARRANGE: Create a vector that does not have a length of 1.
    Salix::Vector3 vec(0.0f, 5.0f, 0.0f);

    // ACT: Call the normalize function.
    vec.normalize();

    // ASSERT: Check that the new vector is a unit vector pointing up.
    CHECK(vec.x == 0.0f);
    CHECK(vec.y == 1.0f);
    CHECK(vec.z == 0.0f);

    // For floating-point numbers, it's best to check they are "approximately" equal.
    // doctest provides a handy Approx() tool for this.
    CHECK(vec.length() == doctest::Approx(1.0f));
}

TEST_CASE("Vector3: Dot Product") {
    // ARRANGE: Create two vectors that are perpendicular (orthogonal).
    Salix::Vector3 forward(0.0f, 0.0f, 1.0f);
    Salix::Vector3 right(1.0f, 0.0f, 0.0f);

    // ACT: Calculate the dot product.
    float result = Salix::dot(forward, right);

    // ASSERT: The dot product of perpendicular vectors should be 0.
    CHECK(result == 0.0f);
}