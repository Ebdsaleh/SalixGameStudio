#include <doctest.h>
#include <Salix/window/WindowConfig.h>
#include <string>
#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

TEST_SUITE("Salix::window::WindowConfigBatch1247") {
    TEST_CASE("window config default title is SalixGameStudio") {
        Salix::WindowConfig config;
        CHECK(config.title == "SalixGameStudio");
    }

    TEST_CASE("window config default dimensions are 1280 by 720") {
        Salix::WindowConfig config;
        CHECK(config.width == 1280);
        CHECK(config.height == 720);
    }

    TEST_CASE("window config title mutation does not alter dimensions") {
        Salix::WindowConfig config;
        config.title = "Editor Preview";
        CHECK(config.title == "Editor Preview");
        CHECK(config.width == 1280);
        CHECK(config.height == 720);
    }

    TEST_CASE("window config width and height remain independently assignable") {
        Salix::WindowConfig config;
        config.width = 1920;
        CHECK(config.width == 1920);
        CHECK(config.height == 720);
        config.height = 1080;
        CHECK(config.width == 1920);
        CHECK(config.height == 1080);
    }

    TEST_CASE("window config preserves explicit zero sized surfaces") {
        Salix::WindowConfig config;
        config.width = 0;
        config.height = 0;
        CHECK(config.width == 0);
        CHECK(config.height == 0);
    }

    TEST_CASE("window config stores negative dimensions without hidden clamping") {
        Salix::WindowConfig config;
        config.width = -640;
        config.height = -480;
        CHECK(config.width == -640);
        CHECK(config.height == -480);
    }

    TEST_CASE("window config copy construction preserves complete value state") {
        Salix::WindowConfig source;
        source.title = "Copied";
        source.width = 800;
        source.height = 600;
        Salix::WindowConfig copy(source);
        CHECK(copy.title == "Copied");
        CHECK(copy.width == 800);
        CHECK(copy.height == 600);
    }

    TEST_CASE("window config copied title storage is independent") {
        Salix::WindowConfig source;
        source.title = "Original";
        Salix::WindowConfig copy = source;
        copy.title = "Copy";
        CHECK(source.title == "Original");
        CHECK(copy.title == "Copy");
    }

    TEST_CASE("window config copy assignment replaces all fields together") {
        Salix::WindowConfig source;
        source.title = "Assigned";
        source.width = 3440;
        source.height = 1440;
        Salix::WindowConfig destination;
        destination.title = "Old";
        destination.width = 1;
        destination.height = 2;
        destination = source;
        CHECK(destination.title == "Assigned");
        CHECK(destination.width == 3440);
        CHECK(destination.height == 1440);
    }

    TEST_CASE("separate window config instances never share mutable state") {
        Salix::WindowConfig first;
        Salix::WindowConfig second;
        first.title = "First";
        first.width = 1024;
        second.title = "Second";
        second.height = 900;
        CHECK(first.title == "First");
        CHECK(first.width == 1024);
        CHECK(first.height == 720);
        CHECK(second.title == "Second");
        CHECK(second.width == 1280);
        CHECK(second.height == 900);
    }
}

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


TEST_SUITE("Salix::window::WindowConfigBatch2047") {
    TEST_CASE("aggregate initialization accepts title width and height together") {
        Salix::WindowConfig config{"Preview", 1600, 900}; CHECK(config.title == "Preview"); CHECK(config.width == 1600); CHECK(config.height == 900);
    }
    TEST_CASE("window title preserves newline and tab characters exactly") {
        Salix::WindowConfig config; config.title = "Line1\nLine2\tTabbed"; CHECK(config.title == "Line1\nLine2\tTabbed");
    }
    TEST_CASE("window title supports long owned strings without altering dimensions") {
        Salix::WindowConfig config; config.title.assign(4096, 'x'); CHECK(config.title.size() == 4096); CHECK(config.width == 1280); CHECK(config.height == 720);
    }
    TEST_CASE("copy assigned config remains independent after source title and dimensions mutate") {
        Salix::WindowConfig source{"Source", 800, 600}; Salix::WindowConfig destination; destination = source;
        source.title = "Changed"; source.width = 1; source.height = 2;
        CHECK(destination.title == "Source"); CHECK(destination.width == 800); CHECK(destination.height == 600);
    }
    TEST_CASE("retained width reference mutates only width") {
        Salix::WindowConfig config; int& width = config.width; width = 2048;
        CHECK(config.width == 2048); CHECK(config.height == 720); CHECK(config.title == "SalixGameStudio");
    }
    TEST_CASE("retained height reference mutates only height") {
        Salix::WindowConfig config; int& height = config.height; height = 1152;
        CHECK(config.height == 1152); CHECK(config.width == 1280); CHECK(config.title == "SalixGameStudio");
    }
    TEST_CASE("minimum width and maximum height integers are stored independently") {
        Salix::WindowConfig config; config.width = (std::numeric_limits<int>::min)(); config.height = (std::numeric_limits<int>::max)();
        CHECK(config.width == (std::numeric_limits<int>::min)()); CHECK(config.height == (std::numeric_limits<int>::max)());
    }
    TEST_CASE("zero width can coexist with a positive height and custom title") {
        Salix::WindowConfig config{"ZeroWidth", 0, 1080}; CHECK(config.title == "ZeroWidth"); CHECK(config.width == 0); CHECK(config.height == 1080);
    }
    TEST_CASE("repeated complete field assignments preserve the final exact state") {
        Salix::WindowConfig config; config.title = "First"; config.width = 1; config.height = 2;
        config.title = "Second"; config.width = -3; config.height = 4;
        config.title = "Final"; config.width = 5120; config.height = 1440;
        CHECK(config.title == "Final"); CHECK(config.width == 5120); CHECK(config.height == 1440);
    }
    TEST_CASE("mutating one config never changes a freshly constructed default config") {
        Salix::WindowConfig mutated; mutated.title = "Mutated"; mutated.width = -1; mutated.height = -2; Salix::WindowConfig fresh;
        CHECK(fresh.title == "SalixGameStudio"); CHECK(fresh.width == 1280); CHECK(fresh.height == 720);
    }
}
