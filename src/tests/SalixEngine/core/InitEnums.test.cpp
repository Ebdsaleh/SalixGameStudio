#include <doctest.h>
#include <Salix/core/InitEnums.h>
#include <array>
#include <set>
#include <type_traits>

TEST_SUITE("Salix::core::InitEnumsBatch1547") {
    TEST_CASE("renderer type neutral value is zero for value initialized startup state") {
        CHECK(static_cast<int>(Salix::RendererType::None) == 0);
    }

    TEST_CASE("renderer type defines five unique startup backend selections") {
        const std::array<Salix::RendererType, 5> values{
            Salix::RendererType::None,
            Salix::RendererType::SDL,
            Salix::RendererType::OpenGL,
            Salix::RendererType::Vulkan,
            Salix::RendererType::DirectX
        };
        std::set<int> unique;
        for (auto value : values) unique.insert(static_cast<int>(value));
        CHECK(unique.size() == values.size());
    }

    TEST_CASE("timer type SDL remains zero default and Chrono remains distinct") {
        CHECK(static_cast<int>(Salix::TimerType::SDL) == 0);
        CHECK(Salix::TimerType::SDL != Salix::TimerType::Chrono);
    }

    TEST_CASE("GUI type neutral startup mode is zero and ImGui is distinct") {
        CHECK(static_cast<int>(Salix::GuiType::None) == 0);
        CHECK(Salix::GuiType::None != Salix::GuiType::ImGui);
    }

    TEST_CASE("application state neutral value is zero for unselected state") {
        CHECK(static_cast<int>(Salix::AppStateType::None) == 0);
    }

    TEST_CASE("application state defines five unique lifecycle destinations") {
        const std::array<Salix::AppStateType, 5> values{
            Salix::AppStateType::None,
            Salix::AppStateType::Launch,
            Salix::AppStateType::Editor,
            Salix::AppStateType::Game,
            Salix::AppStateType::Options
        };
        std::set<int> unique;
        for (auto value : values) unique.insert(static_cast<int>(value));
        CHECK(unique.size() == values.size());
    }

    TEST_CASE("renderer timer GUI and app state remain distinct enum types") {
        CHECK_FALSE((std::is_same_v<Salix::RendererType, Salix::TimerType>));
        CHECK_FALSE((std::is_same_v<Salix::RendererType, Salix::GuiType>));
        CHECK_FALSE((std::is_same_v<Salix::GuiType, Salix::AppStateType>));
    }

    TEST_CASE("renderer selections compare only inside renderer domain") {
        Salix::RendererType renderer = Salix::RendererType::OpenGL;
        CHECK(renderer == Salix::RendererType::OpenGL);
        CHECK(renderer != Salix::RendererType::SDL);
        renderer = Salix::RendererType::Vulkan;
        CHECK(renderer == Salix::RendererType::Vulkan);
    }

    TEST_CASE("application state assignment can represent every current transition target") {
        Salix::AppStateType state = Salix::AppStateType::Launch;
        CHECK(state == Salix::AppStateType::Launch);
        state = Salix::AppStateType::Editor;
        CHECK(state == Salix::AppStateType::Editor);
        state = Salix::AppStateType::Game;
        CHECK(state == Salix::AppStateType::Game);
        state = Salix::AppStateType::Options;
        CHECK(state == Salix::AppStateType::Options);
    }

    TEST_CASE("all initialization enums use integral underlying representations") {
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::RendererType>>));
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::TimerType>>));
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::GuiType>>));
        CHECK((std::is_integral_v<std::underlying_type_t<Salix::AppStateType>>));
    }
}
