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
        CHECK(copy.filename() == "Assets");
        CHECK(copy.parent_path() == Salix::g_project_root_path);
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
            CHECK(Salix::g_project_root_path == std::filesystem::path("Temporary/Root"));
        }
        CHECK(Salix::g_project_root_path == before);
    }
}

TEST_SUITE("Salix::core::CoreBatch6447") {
    TEST_CASE("batch6447 project root preserves a single filename component") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = "Batch6447";
        CHECK(Salix::g_project_root_path.filename() == "Batch6447");
        CHECK(Salix::g_project_root_path.parent_path().empty());
    }

    TEST_CASE("batch6447 project root append creates expected child component") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = "Root6447";
        Salix::g_project_root_path /= "Assets";
        CHECK(Salix::g_project_root_path.filename() == "Assets");
        CHECK(Salix::g_project_root_path.parent_path().filename() == "Root6447");
    }

    TEST_CASE("batch6447 project root replacement discards prior child components") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Old6447") / "Assets";
        Salix::g_project_root_path = "New6447";
        CHECK(Salix::g_project_root_path == std::filesystem::path("New6447"));
    }

    TEST_CASE("batch6447 project root preserves dot component text before normalization") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Root6447") / "." / "Game";
        CHECK(Salix::g_project_root_path != Salix::g_project_root_path.lexically_normal());
        CHECK(Salix::g_project_root_path.lexically_normal() == std::filesystem::path("Root6447") / "Game");
    }

    TEST_CASE("batch6447 copied root can be cleared without clearing global") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Root6447") / "Game";
        auto copy = Salix::g_project_root_path;
        copy.clear();
        CHECK(copy.empty());
        CHECK_FALSE(Salix::g_project_root_path.empty());
    }

    TEST_CASE("batch6447 global root may be cleared after populated assignment") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Root6447") / "Game";
        Salix::g_project_root_path.clear();
        CHECK(Salix::g_project_root_path.empty());
    }

    TEST_CASE("batch6447 extensionless root reports empty extension") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / "Batch6447";
        CHECK(Salix::g_project_root_path.extension().empty());
        CHECK(Salix::g_project_root_path.stem() == "Batch6447");
    }

    TEST_CASE("batch6447 dot extension root preserves full filename") {
        Batch1647ProjectRootGuard guard;
        Salix::g_project_root_path = std::filesystem::path("Projects") / "Batch6447.realm";
        CHECK(Salix::g_project_root_path.extension() == ".realm");
        CHECK(Salix::g_project_root_path.filename() == "Batch6447.realm");
    }

    TEST_CASE("batch6447 retained reference sees subsequent complete assignment") {
        Batch1647ProjectRootGuard guard;
        auto& retained = Salix::g_project_root_path;
        Salix::g_project_root_path = "Before6447";
        Salix::g_project_root_path = "After6447";
        CHECK(retained == std::filesystem::path("After6447"));
    }

    TEST_CASE("batch6447 guard restores nested scope root after multiple mutations") {
        const auto original = Salix::g_project_root_path;
        {
            Batch1647ProjectRootGuard guard;
            Salix::g_project_root_path = "One6447";
            Salix::g_project_root_path /= "Two6447";
            Salix::g_project_root_path.clear();
            Salix::g_project_root_path = "Final6447";
        }
        CHECK(Salix::g_project_root_path == original);
    }
}
