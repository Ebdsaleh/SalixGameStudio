#include <doctest.h>
#include <Salix/window/WindowConfig.h>
#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

TEST_SUITE("Salix::window::WindowConfigClosure1747") {
    TEST_CASE("batch1747 window config preserves an empty title without modifying dimensions") {
        Salix::WindowConfig config;
        config.title.clear();
        CHECK(config.title.empty());
        CHECK(config.width == 1280);
        CHECK(config.height == 720);
    }

    TEST_CASE("batch1747 window config preserves embedded NUL bytes in title storage") {
        Salix::WindowConfig config;
        config.title = std::string("Main\0Preview", 12);
        CHECK(config.title.size() == 12);
        CHECK(config.title[4] == '\0');
        CHECK(config.width == 1280);
    }

    TEST_CASE("batch1747 window config preserves UTF8 title bytes exactly") {
        Salix::WindowConfig config;
        config.title = u8"Salix Ω 世界";
        CHECK(config.title == u8"Salix Ω 世界");
    }

    TEST_CASE("batch1747 window config stores integer dimension extrema without clamping") {
        Salix::WindowConfig config;
        config.width = std::numeric_limits<int>::min();
        config.height = std::numeric_limits<int>::max();
        CHECK(config.width == std::numeric_limits<int>::min());
        CHECK(config.height == std::numeric_limits<int>::max());
    }

    TEST_CASE("batch1747 window config self assignment preserves all fields") {
        Salix::WindowConfig config;
        config.title = "Self";
        config.width = -1;
        config.height = 9999;
        config = config;
        CHECK(config.title == "Self");
        CHECK(config.width == -1);
        CHECK(config.height == 9999);
    }

    TEST_CASE("batch1747 window config move construction transfers title and dimensions") {
        Salix::WindowConfig source;
        source.title = "Moved";
        source.width = 800;
        source.height = 450;
        Salix::WindowConfig moved(std::move(source));
        CHECK(moved.title == "Moved");
        CHECK(moved.width == 800);
        CHECK(moved.height == 450);
    }

    TEST_CASE("batch1747 window config move assignment replaces destination state") {
        Salix::WindowConfig source;
        source.title = "MoveAssigned";
        source.width = 5120;
        source.height = 1440;
        Salix::WindowConfig destination;
        destination.title = "Old";
        destination = std::move(source);
        CHECK(destination.title == "MoveAssigned");
        CHECK(destination.width == 5120);
        CHECK(destination.height == 1440);
    }

    TEST_CASE("batch1747 window config swapping values exchanges title width and height together") {
        Salix::WindowConfig a, b;
        a.title = "A"; a.width = 1; a.height = 2;
        b.title = "B"; b.width = 3; b.height = 4;
        std::swap(a, b);
        CHECK(a.title == "B");
        CHECK(a.width == 3);
        CHECK(a.height == 4);
        CHECK(b.title == "A");
        CHECK(b.width == 1);
        CHECK(b.height == 2);
    }

    TEST_CASE("batch1747 window config vector copies preserve ordering and independent titles") {
        Salix::WindowConfig a, b;
        a.title = "First";
        b.title = "Second";
        std::vector<Salix::WindowConfig> values{a, b};
        REQUIRE(values.size() == 2);
        values[0].title = "Changed";
        CHECK(values[0].title == "Changed");
        CHECK(values[1].title == "Second");
        CHECK(a.title == "First");
    }

    TEST_CASE("batch1747 window config retained title reference mutates only its owning object") {
        Salix::WindowConfig first, second;
        std::string& title = first.title;
        title = "Referenced";
        CHECK(first.title == "Referenced");
        CHECK(second.title == "SalixGameStudio");
    }
}
