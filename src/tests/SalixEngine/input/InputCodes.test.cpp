#include <doctest.h>
#include <Salix/input/InputCodes.h>
#include <array>
#include <map>
#include <set>
#include <string>
#include <type_traits>

TEST_SUITE("Salix::input::InputCodesBatch1547") {
    TEST_CASE("key code none is distinct from representative usable keys") {
        CHECK(Salix::KeyCode::None != Salix::KeyCode::A);
        CHECK(Salix::KeyCode::None != Salix::KeyCode::Space);
        CHECK(Salix::KeyCode::None != Salix::KeyCode::Escape);
    }

    TEST_CASE("alphabetic key identifiers are all unique") {
        const std::array<Salix::KeyCode, 26> keys{
            Salix::KeyCode::A, Salix::KeyCode::B, Salix::KeyCode::C, Salix::KeyCode::D,
            Salix::KeyCode::E, Salix::KeyCode::F, Salix::KeyCode::G, Salix::KeyCode::H,
            Salix::KeyCode::I, Salix::KeyCode::J, Salix::KeyCode::K, Salix::KeyCode::L,
            Salix::KeyCode::M, Salix::KeyCode::N, Salix::KeyCode::O, Salix::KeyCode::P,
            Salix::KeyCode::Q, Salix::KeyCode::R, Salix::KeyCode::S, Salix::KeyCode::T,
            Salix::KeyCode::U, Salix::KeyCode::V, Salix::KeyCode::W, Salix::KeyCode::X,
            Salix::KeyCode::Y, Salix::KeyCode::Z
        };
        std::set<Salix::KeyCode> unique(keys.begin(), keys.end());
        CHECK(unique.size() == keys.size());
    }

    TEST_CASE("numeric row key identifiers are all unique") {
        const std::array<Salix::KeyCode, 10> keys{
            Salix::KeyCode::Alpha0, Salix::KeyCode::Alpha1, Salix::KeyCode::Alpha2,
            Salix::KeyCode::Alpha3, Salix::KeyCode::Alpha4, Salix::KeyCode::Alpha5,
            Salix::KeyCode::Alpha6, Salix::KeyCode::Alpha7, Salix::KeyCode::Alpha8,
            Salix::KeyCode::Alpha9
        };
        std::set<Salix::KeyCode> unique(keys.begin(), keys.end());
        CHECK(unique.size() == keys.size());
    }

    TEST_CASE("left and right modifier key identifiers remain distinct") {
        const std::array<Salix::KeyCode, 8> modifiers{
            Salix::KeyCode::LeftShift, Salix::KeyCode::RightShift,
            Salix::KeyCode::LeftControl, Salix::KeyCode::RightControl,
            Salix::KeyCode::LeftAlt, Salix::KeyCode::RightAlt,
            Salix::KeyCode::LeftGui, Salix::KeyCode::RightGui
        };
        std::set<Salix::KeyCode> unique(modifiers.begin(), modifiers.end());
        CHECK(unique.size() == modifiers.size());
        CHECK(Salix::KeyCode::LeftShift != Salix::KeyCode::RightShift);
        CHECK(Salix::KeyCode::LeftControl != Salix::KeyCode::RightControl);
        CHECK(Salix::KeyCode::LeftAlt != Salix::KeyCode::RightAlt);
    }

    TEST_CASE("function key identifiers F1 through F12 are all unique") {
        const std::array<Salix::KeyCode, 12> keys{
            Salix::KeyCode::F1, Salix::KeyCode::F2, Salix::KeyCode::F3, Salix::KeyCode::F4,
            Salix::KeyCode::F5, Salix::KeyCode::F6, Salix::KeyCode::F7, Salix::KeyCode::F8,
            Salix::KeyCode::F9, Salix::KeyCode::F10, Salix::KeyCode::F11, Salix::KeyCode::F12
        };
        std::set<Salix::KeyCode> unique(keys.begin(), keys.end());
        CHECK(unique.size() == keys.size());
    }

    TEST_CASE("navigation and editing key identifiers do not alias one another") {
        const std::array<Salix::KeyCode, 9> keys{
            Salix::KeyCode::Up, Salix::KeyCode::Down, Salix::KeyCode::Left, Salix::KeyCode::Right,
            Salix::KeyCode::Tab, Salix::KeyCode::Delete, Salix::KeyCode::BackSpace,
            Salix::KeyCode::ContextMenu, Salix::KeyCode::PrintScreen
        };
        std::set<Salix::KeyCode> unique(keys.begin(), keys.end());
        CHECK(unique.size() == keys.size());
    }

    TEST_CASE("punctuation key identifiers remain individually addressable") {
        const std::array<Salix::KeyCode, 10> keys{
            Salix::KeyCode::BackSlash, Salix::KeyCode::Comma, Salix::KeyCode::Separator,
            Salix::KeyCode::Slash, Salix::KeyCode::Apostrophe, Salix::KeyCode::SemiColon,
            Salix::KeyCode::Period, Salix::KeyCode::Minus, Salix::KeyCode::Equals,
            Salix::KeyCode::CapsLock
        };
        std::set<Salix::KeyCode> unique(keys.begin(), keys.end());
        CHECK(unique.size() == keys.size());
    }

    TEST_CASE("mouse button identifiers include distinct none left middle and right states") {
        const std::array<Salix::MouseButton, 4> buttons{
            Salix::MouseButton::None,
            Salix::MouseButton::Left,
            Salix::MouseButton::Middle,
            Salix::MouseButton::Right
        };
        std::set<Salix::MouseButton> unique(buttons.begin(), buttons.end());
        CHECK(unique.size() == buttons.size());
        CHECK(Salix::MouseButton::None != Salix::MouseButton::Left);
    }

    TEST_CASE("mouse scroll identifiers keep neutral forward and backward states distinct") {
        const std::array<Salix::MouseScroll, 3> directions{
            Salix::MouseScroll::None,
            Salix::MouseScroll::Forward,
            Salix::MouseScroll::Backward
        };
        std::set<Salix::MouseScroll> unique(directions.begin(), directions.end());
        CHECK(unique.size() == directions.size());
        CHECK(Salix::MouseScroll::Forward != Salix::MouseScroll::Backward);
    }

    TEST_CASE("input code domains remain strongly typed and usable as ordered lookup keys") {
        CHECK_FALSE((std::is_same_v<Salix::KeyCode, Salix::MouseButton>));
        CHECK_FALSE((std::is_same_v<Salix::KeyCode, Salix::MouseScroll>));
        CHECK_FALSE((std::is_same_v<Salix::MouseButton, Salix::MouseScroll>));

        std::map<Salix::KeyCode, const char*> labels;
        labels[Salix::KeyCode::Escape] = "Escape";
        labels[Salix::KeyCode::Space] = "Space";
        CHECK(std::string(labels.at(Salix::KeyCode::Escape)) == "Escape");
        CHECK(std::string(labels.at(Salix::KeyCode::Space)) == "Space");
    }
}
