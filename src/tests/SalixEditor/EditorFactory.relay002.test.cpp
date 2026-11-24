// CRI relay 002: EditorFactory; off-screen and observable behavior.
#include "CriRelay002Fixtures.h"
#include <Editor/states/EditorState.h>
#include <Editor/reflection/ui/TypeDrawer.h>
#include <Salix/states/IAppState.h>
namespace {
using namespace EditorCri;
std::unique_ptr<IAppState> create(AppStateType t){return std::unique_ptr<IAppState>(create_editor_state(t));}
}
TEST_SUITE("EditorCRI002_EditorFactory") {
    TEST_CASE("CRI002 EditorFactory: editor request creates actual EditorState") {
        auto p=create(AppStateType::Editor);REQUIRE(p);CHECK(dynamic_cast<EditorState*>(p.get())!=nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: none request returns null") {
        CHECK(create(AppStateType::None)==nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: launch request returns null") {
        CHECK(create(AppStateType::Launch)==nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: game request returns null") {
        CHECK(create(AppStateType::Game)==nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: options request returns null") {
        CHECK(create(AppStateType::Options)==nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: unknown enum request returns null") {
        CHECK(create(static_cast<AppStateType>(999))==nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: two live factory states are independent allocations") {
        auto a=create(AppStateType::Editor);auto b=create(AppStateType::Editor);REQUIRE(a);REQUIRE(b);CHECK(a.get()!=b.get());a.reset();CHECK(dynamic_cast<EditorState*>(b.get())!=nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: factory state supports virtual teardown before entry") {
        auto p=create(AppStateType::Editor);REQUIRE(p);p->on_exit();p->render(nullptr);CHECK(dynamic_cast<EditorState*>(p.get())!=nullptr);
    }

    TEST_CASE("CRI002 EditorFactory: bound offscreen context is used by actual DLL drawing") {
        GuiFrame g;Property p{};p.name="Value";p.type=PropertyType::Int;auto n=YAML::Load("{Value: 4}");CHECK(g.draw([&]{TypeDrawer::draw_yaml_property(p,n);})>0);CHECK(n["Value"].as<int>()==4);
    }

    TEST_CASE("CRI002 EditorFactory: DLL context can detach then rebind without changing data") {
        GuiFrame g;set_imgui_context(nullptr);set_imgui_context(g.current);Property p{};p.name="Value";p.type=PropertyType::Bool;auto n=YAML::Load("{Value: true}");CHECK(g.draw([&]{TypeDrawer::draw_yaml_property(p,n);})>0);CHECK(n["Value"].as<bool>());
    }
}
