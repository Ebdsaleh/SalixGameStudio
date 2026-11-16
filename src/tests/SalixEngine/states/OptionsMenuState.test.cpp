#define IMGUI_DEFINE_MATH_OPERATORS
#include <doctest.h>
#include <Salix/states/OptionsMenuState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace {

struct Batch7247OptionsFrame {
    ImGuiContext* previous = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();

    Batch7247OptionsFrame() {
        ImGui::SetCurrentContext(owned);
        Salix::ImGuiInputManager::set_context_for_testing(owned);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1280.0f, 720.0f);
        io.DeltaTime = 1.0f / 60.0f;
        if (io.Fonts->Fonts.empty()) {
            io.Fonts->AddFontDefault();
        }
        io.Fonts->Build();
    }

    ~Batch7247OptionsFrame() {
        ImGui::SetCurrentContext(owned);
        Salix::ImGuiInputManager::set_context_for_testing(previous_engine);
        ImGui::DestroyContext(owned);
        ImGui::SetCurrentContext(previous);
    }

    template<typename Fn>
    void run(Fn&& fn) {
        ImGui::SetCurrentContext(owned);
        ImGui::NewFrame();
        fn();
        ImGui::EndFrame();
    }

    template<typename Fn>
    bool activate_button(const char* window_name, const char* label, Fn&& fn) {
        // Prime the real state UI once so the named ImGui window exists.
        run(fn);

        ImGui::SetCurrentContext(owned);
        ImGuiWindow* window = ImGui::FindWindowByName(window_name);
        if (!window) {
            return false;
        }

        const ImGuiID id = window->GetID(label);
        if (id == 0) {
            return false;
        }

        // State tests own the response to ImGui::Button(), not SDL/backend
        // hit-testing. Use Dear ImGui's programmatic activation path to make
        // the real button report pressed on the next frame.
        ImGui::ActivateItemByID(id);
        ImGui::NewFrame();
        fn();
        ImGui::EndFrame();
        return true;
    }
};

Salix::InitContext batch7247_options_context(
    MockEngineInterface& engine,
    Salix::IGui* gui = nullptr) {
    Salix::InitContext context{};
    context.engine = &engine;
    context.gui = gui;
    return context;
}

} // namespace

TEST_SUITE("Salix::states::OptionsMenuStateBatch7247") {
    TEST_CASE("batch7247 options state construction and destruction are safe") {
        Salix::OptionsMenuState state;
        CHECK(true);
    }

    TEST_CASE("batch7247 options on enter sets options engine mode") {
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        CHECK(engine.current_mode == Salix::EngineMode::Options);
        CHECK(engine.set_mode_calls == 1);
    }

    TEST_CASE("batch7247 options on enter works with null GUI") {
        MockEngineInterface engine;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine));
        CHECK(engine.current_mode == Salix::EngineMode::Options);
    }

    TEST_CASE("batch7247 repeated options on enter repeats explicit mode selection") {
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        const auto context = batch7247_options_context(engine, &gui);
        state.on_enter(context);
        state.on_enter(context);
        CHECK(engine.set_mode_calls == 2);
        CHECK(engine.current_mode == Salix::EngineMode::Options);
    }

    TEST_CASE("batch7247 options on exit is safe after entry") {
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        CHECK_NOTHROW(state.on_exit());
    }

    TEST_CASE("batch7247 options render is safe with null renderer") {
        Salix::OptionsMenuState state;
        CHECK_NOTHROW(state.render(nullptr));
    }

    TEST_CASE("batch7247 options render is safe with mock renderer") {
        Salix::OptionsMenuState state;
        MockIRenderer renderer;
        CHECK_NOTHROW(state.render(&renderer));
        CHECK(renderer.begin_frame_calls == 0);
        CHECK(renderer.end_frame_calls == 0);
    }

    TEST_CASE("batch7247 null GUI update safely skips menu construction") {
        MockEngineInterface engine;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine));
        CHECK_NOTHROW(state.update(0.0f));
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 repeated null GUI updates remain inert") {
        MockEngineInterface engine;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine));
        state.update(-1.0f);
        state.update(7247.0f);
        CHECK(engine.switch_state_calls == 0);
        CHECK(engine.current_mode == Salix::EngineMode::Options);
    }

    TEST_CASE("batch7247 idle options frame does not switch state") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 negative delta options frame remains stable") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(-7247.0f); });
        CHECK(engine.switch_state_calls == 0);
        CHECK(engine.running);
    }

    TEST_CASE("batch7247 large delta options frame remains stable") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(7247.0f); });
        CHECK(engine.switch_state_calls == 0);
        CHECK(engine.current_mode == Salix::EngineMode::Options);
    }

    TEST_CASE("batch7247 options UI registers Back button in named ImGui window") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });

        ImGui::SetCurrentContext(frame.owned);
        ImGuiWindow* window = ImGui::FindWindowByName("Options Menu");
        REQUIRE(window != nullptr);
        CHECK(window->GetID("Back to Launcher") != 0);
    }

    TEST_CASE("batch7247 options UI leaves engine running without input") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.running);
    }

    TEST_CASE("batch7247 options UI does not delegate rendering through GUI interface") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.render_calls == 0);
        CHECK(gui.platform_window_calls == 0);
    }

    TEST_CASE("batch7247 options UI does not display file dialogs") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.display_dialog_calls == 0);
        CHECK(gui.create_dialog_calls == 0);
    }

    TEST_CASE("batch7247 back button click switches to launcher") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        REQUIRE(frame.activate_button("Options Menu", "Back to Launcher", [&] { state.update(0.0f); }));
        CHECK(engine.current_state == Salix::AppStateType::Launch);
    }

    TEST_CASE("batch7247 back button click performs one switch during release frame") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        REQUIRE(frame.activate_button("Options Menu", "Back to Launcher", [&] { state.update(0.0f); }));
        CHECK(engine.switch_state_calls == 1);
    }

    TEST_CASE("batch7247 back button transition does not stop engine") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        REQUIRE(frame.activate_button("Options Menu", "Back to Launcher", [&] { state.update(0.0f); }));
        CHECK(engine.running);
    }

    TEST_CASE("batch7247 back button transition preserves options mode until engine changes state") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        REQUIRE(frame.activate_button("Options Menu", "Back to Launcher", [&] { state.update(0.0f); }));
        CHECK(engine.current_mode == Salix::EngineMode::Options);
        CHECK(engine.current_state == Salix::AppStateType::Launch);
    }

    TEST_CASE("batch7247 idle frame after entry keeps options mode") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.current_mode == Salix::EngineMode::Options);
    }

    TEST_CASE("batch7247 two idle frames produce no transition") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 options update does not mutate engine time scale") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        engine.set_time_scale(7.247f);
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.get_time_scale() == doctest::Approx(7.247f));
    }

    TEST_CASE("batch7247 options render leaves mock renderer state untouched") {
        MockIRenderer renderer;
        Salix::OptionsMenuState state;
        state.render(&renderer);
        CHECK_FALSE(renderer.initialized);
        CHECK(renderer.clear_calls == 0);
        CHECK(renderer.draw_sprite_call_count == 0);
    }

    TEST_CASE("batch7247 options lifecycle enter update exit remains safe") {
        Batch7247OptionsFrame frame;
        MockEngineInterface engine;
        MockIGui gui;
        Salix::OptionsMenuState state;
        state.on_enter(batch7247_options_context(engine, &gui));
        frame.run([&] { state.update(1.0f / 60.0f); });
        state.on_exit();
        CHECK(engine.current_mode == Salix::EngineMode::Options);
        CHECK(engine.switch_state_calls == 0);
    }
}
