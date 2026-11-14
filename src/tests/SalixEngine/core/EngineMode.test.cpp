#include <doctest.h>
#include <Salix/core/EngineMode.h>
#include <array>
#include <set>
#include <type_traits>

TEST_SUITE("Salix::core::EngineModeBatch1647") {
    TEST_CASE("value initialized engine mode is None") {
        Salix::EngineMode mode{};
        CHECK(mode == Salix::EngineMode::None);
    }

    TEST_CASE("all current engine modes are distinct") {
        const std::array<Salix::EngineMode, 6> modes{
            Salix::EngineMode::None,
            Salix::EngineMode::Launch,
            Salix::EngineMode::Editor,
            Salix::EngineMode::Game,
            Salix::EngineMode::Options,
            Salix::EngineMode::Debug
        };
        std::set<Salix::EngineMode> unique(modes.begin(), modes.end());
        CHECK(unique.size() == modes.size());
    }

    TEST_CASE("launch mode remains distinct from editor and game") {
        CHECK(Salix::EngineMode::Launch != Salix::EngineMode::Editor);
        CHECK(Salix::EngineMode::Launch != Salix::EngineMode::Game);
    }

    TEST_CASE("editor and game modes remain independently assignable") {
        Salix::EngineMode mode = Salix::EngineMode::Editor;
        CHECK(mode == Salix::EngineMode::Editor);
        mode = Salix::EngineMode::Game;
        CHECK(mode == Salix::EngineMode::Game);
    }

    TEST_CASE("options and debug modes remain independently assignable") {
        Salix::EngineMode mode = Salix::EngineMode::Options;
        CHECK(mode == Salix::EngineMode::Options);
        mode = Salix::EngineMode::Debug;
        CHECK(mode == Salix::EngineMode::Debug);
    }

    TEST_CASE("engine mode is strongly typed and not implicitly convertible to int") {
        CHECK_FALSE((std::is_convertible_v<Salix::EngineMode, int>));
        CHECK((std::is_enum_v<Salix::EngineMode>));
    }

    TEST_CASE("engine mode underlying representation is integral") {
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::EngineMode>>));
    }

    TEST_CASE("engine mode copy construction preserves exact selected mode") {
        const Salix::EngineMode source = Salix::EngineMode::Debug;
        const Salix::EngineMode copy = source;
        CHECK(copy == source);
    }

    TEST_CASE("engine mode copy assignment replaces previous mode") {
        Salix::EngineMode destination = Salix::EngineMode::Launch;
        destination = Salix::EngineMode::Options;
        CHECK(destination == Salix::EngineMode::Options);
    }

    TEST_CASE("engine mode can be used as an ordered set key without aliasing values") {
        std::set<Salix::EngineMode> modes;
        modes.insert(Salix::EngineMode::Game);
        modes.insert(Salix::EngineMode::Game);
        modes.insert(Salix::EngineMode::Editor);
        CHECK(modes.size() == 2);
        CHECK(modes.count(Salix::EngineMode::Game) == 1);
        CHECK(modes.count(Salix::EngineMode::Editor) == 1);
    }
}

TEST_SUITE("Salix::core::EngineModeBatch6347") {
    TEST_CASE("batch6347 None underlying value remains zero") {
        CHECK(static_cast<int>(Salix::EngineMode::None) == 0);
    }

    TEST_CASE("batch6347 Launch underlying value follows None") {
        CHECK(static_cast<int>(Salix::EngineMode::Launch) == 1);
    }

    TEST_CASE("batch6347 Editor underlying value remains two") {
        CHECK(static_cast<int>(Salix::EngineMode::Editor) == 2);
    }

    TEST_CASE("batch6347 Game underlying value remains three") {
        CHECK(static_cast<int>(Salix::EngineMode::Game) == 3);
    }

    TEST_CASE("batch6347 Options underlying value remains four") {
        CHECK(static_cast<int>(Salix::EngineMode::Options) == 4);
    }

    TEST_CASE("batch6347 Debug underlying value remains five") {
        CHECK(static_cast<int>(Salix::EngineMode::Debug) == 5);
    }

    TEST_CASE("batch6347 declared modes remain a contiguous zero based sequence") {
        const std::array<Salix::EngineMode, 6> modes{
            Salix::EngineMode::None, Salix::EngineMode::Launch, Salix::EngineMode::Editor,
            Salix::EngineMode::Game, Salix::EngineMode::Options, Salix::EngineMode::Debug
        };
        for (std::size_t i = 0; i < modes.size(); ++i) {
            CHECK(static_cast<std::size_t>(modes[i]) == i);
        }
    }

    TEST_CASE("batch6347 ordered set stores all declared modes exactly once") {
        const std::set<Salix::EngineMode> modes{
            Salix::EngineMode::None, Salix::EngineMode::Launch, Salix::EngineMode::Editor,
            Salix::EngineMode::Game, Salix::EngineMode::Options, Salix::EngineMode::Debug
        };
        CHECK(modes.size() == 6);
    }

    TEST_CASE("batch6347 roundtrip through underlying type preserves every mode") {
        using U = std::underlying_type_t<Salix::EngineMode>;
        const std::array<Salix::EngineMode, 6> modes{
            Salix::EngineMode::None, Salix::EngineMode::Launch, Salix::EngineMode::Editor,
            Salix::EngineMode::Game, Salix::EngineMode::Options, Salix::EngineMode::Debug
        };
        for (const auto mode : modes) {
            CHECK(static_cast<Salix::EngineMode>(static_cast<U>(mode)) == mode);
        }
    }

    TEST_CASE("batch6347 pass by value preserves selected mode independently") {
        const auto identity = [](Salix::EngineMode mode) { return mode; };
        CHECK(identity(Salix::EngineMode::Debug) == Salix::EngineMode::Debug);
        CHECK(identity(Salix::EngineMode::None) == Salix::EngineMode::None);
    }
}
