#include <doctest.h>
#include <Salix/math/Point.h>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <limits>
#include <sstream>
#include <type_traits>
#include <utility>

TEST_SUITE("Salix::math::PointClosure1747") {
    TEST_CASE("batch1747 point remains an aggregate with standard layout") {
        CHECK((std::is_aggregate_v<Salix::Point>));
        CHECK((std::is_standard_layout_v<Salix::Point>));
    }

    TEST_CASE("batch1747 point remains trivially copyable for lightweight coordinate transport") {
        CHECK((std::is_trivially_copyable_v<Salix::Point>));
    }

    TEST_CASE("batch1747 point structured binding exposes copied X and Y values") {
        const Salix::Point point{-12, 34};
        const auto [x, y] = point;
        CHECK(x == -12);
        CHECK(y == 34);
    }

    TEST_CASE("batch1747 point equality is transitive across three separately created values") {
        const Salix::Point a{-4, 9}, b{-4, 9}, c{-4, 9};
        CHECK(a == b);
        CHECK(b == c);
        CHECK(a == c);
    }

    TEST_CASE("batch1747 point inequality remains exact complement of equality after one axis mutation") {
        Salix::Point a{8, 13};
        Salix::Point b = a;
        CHECK(a == b);
        b.y += 1;
        CHECK(a != b);
        CHECK_FALSE(a == b);
    }

    TEST_CASE("batch1747 point swapping two values exchanges both coordinates atomically") {
        Salix::Point a{1, 2}, b{-3, -4};
        std::swap(a, b);
        CHECK((a == Salix::Point{-3, -4}));
        CHECK((b == Salix::Point{1, 2}));
    }

    TEST_CASE("batch1747 point stream prints signed integer extrema without losing labels") {
        std::ostringstream out;
        out << Salix::Point{std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};
        CHECK(out.str().find("x: " + std::to_string(std::numeric_limits<int>::min())) != std::string::npos);
        CHECK(out.str().find("y: " + std::to_string(std::numeric_limits<int>::max())) != std::string::npos);
    }

    TEST_CASE("batch1747 point JSON archive restores extrema into a prepopulated destination") {
        const Salix::Point source{std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};
        std::stringstream stream;
        { cereal::JSONOutputArchive out(stream); out(source); }
        Salix::Point target{7, 8};
        { cereal::JSONInputArchive in(stream); in(target); }
        CHECK(target == source);
    }

    TEST_CASE("batch1747 point binary archive is deterministic for identical coordinate values") {
        const Salix::Point source{-123456, 654321};
        std::stringstream first, second;
        { cereal::BinaryOutputArchive out(first); out(source); }
        { cereal::BinaryOutputArchive out(second); out(source); }
        CHECK(first.str() == second.str());
    }

    TEST_CASE("batch1747 point self assignment retains both coordinate fields") {
        Salix::Point point{-17, 29};
        point = point;
        CHECK((point == Salix::Point{-17, 29}));
    }
}
