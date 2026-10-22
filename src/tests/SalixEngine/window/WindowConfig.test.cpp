#include <doctest.h>
#include <Salix/window/WindowConfig.h>
#include <string>

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
