#include <doctest.h>
#include <Salix/core/Core.h>
#include <filesystem>
#include <string>

namespace {
struct Batch1647ProjectRootGuard {
    std::filesystem::path original = Salix::g_project_root_path;
    ~Batch1647ProjectRootGuard() { Salix::g_project_root_path = original; }
};
}

TEST_SUITE("Salix::core::CoreBatch1647") {
    TEST_CASE("project root global accepts an empty path") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path.clear();
        CHECK(Salix::g_project_root_path.empty());
    }

    TEST_CASE("project root global preserves a relative project directory") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / "Example";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Projects") / "Example");
    }

    TEST_CASE("project root global preserves spaces in path components") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("My Projects") / "Space Game";
        CHECK(Salix::g_project_root_path.filename() == "Space Game");
        CHECK(Salix::g_project_root_path.parent_path().filename() == "My Projects");
    }

    TEST_CASE("project root global replacement overwrites complete previous value") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = "First/Project";
        Salix::g_project_root_path = "Second/Project";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Second/Project"));
    }

    TEST_CASE("project root global reference remains stable across assignments") {
        Batch1647ProjectRootGuard guard;
        auto* address = &Salix::g_project_root_path;
        Salix::g_project_root_path = "One";
        Salix::g_project_root_path = "Two";
        CHECK(&Salix::g_project_root_path == address);
    }

    TEST_CASE("project root global can be mutated through a retained reference") {
        Batch1647ProjectRootGuard guard;
        auto& root = Salix::g_project_root_path;
        root = "Referenced/Project";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Referenced/Project"));
    }

    TEST_CASE("copying project root path creates independent value storage") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = "Source/Project";
        auto copy = Salix::g_project_root_path;
        copy /= "Assets";
        CHECK(Salix::g_project_root_path == std::filesystem::path("Source/Project"));
        CHECK(copy == std::filesystem::path("Source/Project/Assets"));
    }

    TEST_CASE("project root global does not silently lexically normalize parent traversal") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / ".." / "Game";
        CHECK(Salix::g_project_root_path != Salix::g_project_root_path.lexically_normal());
        CHECK(Salix::g_project_root_path.lexically_normal() == std::filesystem::path("Game"));
    }

    TEST_CASE("project root global retains project file extension when assigned a file path") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / "demo.salixproj";
        CHECK(Salix::g_project_root_path.extension() == ".salixproj");
        CHECK(Salix::g_project_root_path.stem() == "demo");
    }

    TEST_CASE("sequential project root mutations can return to original guarded value") {
        const auto before = Salix::g_project_root_path;
        {
            Batch1647ProjectRootGuard guard;
            Salix::g_project_root_path = "Temporary/Root";
            CHECK(Salix::g_project_root_path != before);
        }
        CHECK(Salix::g_project_root_path == before);
    }
}
