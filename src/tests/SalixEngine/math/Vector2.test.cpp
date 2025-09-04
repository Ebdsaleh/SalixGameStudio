// Tests/math/Vector2.test.cpp
#include <doctest.h>
#include <Salix/math/Vector2.h>


TEST_CASE("Vector2 Addition") {
    // ARRANGE: Create two vectors we want to add.
    Salix::Vector2 a(5.0f, 1.0f);
    Salix::Vector2 b(4.0f, 5.0f);

    // ACT: Perform the addition.
    Salix::Vector2 result = a + b;

    // ASSERT: Check if each component of the result is correct.
    CHECK(result.x == 9.f);
    CHECK(result.y == 6.0f);
}

TEST_CASE("Vector2: Normalization") {
    // ARRANGE: Create a vector that does not have a length of 1.
    Salix::Vector2 vec(0.0f, 5.0f);

    // ACT: Call the normalize function.
    vec.normalize();

    // ASSERT: Check that the new vector is a unit vector pointing up.
    CHECK(vec.x == 0.0f);
    CHECK(vec.y == 1.0f);
 

    // For floating-point numbers, it's best to check they are "approximately" equal.
    // doctest provides a handy Approx() tool for this.
    CHECK(vec.length() == doctest::Approx(1.0f));
}


TEST_CASE("Vector2: Dot Product") {
    // ARRANGE: Create two vectors that are perpendicular (orthogonal).
    Salix::Vector2 forward(2.0f, 1.0f);
    Salix::Vector2 right(1.0f, 1.0f);

    // ACT: Calculate the dot product.
    float result = Salix::dot(forward, right);

    // ASSERT: The dot product of perpendicular vectors should be 0.
    CHECK(result == 3.0f);
}

TEST_CASE("Vector2: Dot Product of Perpendicular Vectors is Zero") {
    // ARRANGE: Create two vectors that are exactly perpendicular.
    Salix::Vector2 up(0.0f, 1.0f);
    Salix::Vector2 right(1.0f, 0.0f);

    // ACT: Calculate the dot product.
    float result = Salix::dot(up, right);

    // ASSERT: The dot product of perpendicular vectors must be 0.
    // (0.0 * 1.0) + (1.0 * 0.0) = 0
    CHECK(result == 0.0f);
}

TEST_CASE("Vector2: Dot Product of Parallel Vectors is Positive") {
    // ARRANGE: Create two vectors pointing in the same direction.
    Salix::Vector2 a(2.0f, 3.0f);
    Salix::Vector2 b(4.0f, 6.0f);

    // ACT: Calculate the dot product.
    float result = Salix::dot(a, b);

    // ASSERT: The result should be positive.
    // (2.0 * 4.0) + (3.0 * 6.0) = 8 + 18 = 26
    CHECK(result > 0.0f);
    CHECK(result == 26.0f);
}

TEST_CASE("Vector2: Dot Product of Opposite Vectors is Negative") {
    // ARRANGE: Create two vectors pointing in opposite directions.
    Salix::Vector2 a(2.0f, 3.0f);
    Salix::Vector2 b(-2.0f, -3.0f);

    // ACT: Calculate the dot product.
    float result = Salix::dot(a, b);

    // ASSERT: The result should be negative.
    // (2.0 * -2.0) + (3.0 * -3.0) = -4 + -9 = -13
    CHECK(result < 0.0f);
    CHECK(result == -13.0f);
}