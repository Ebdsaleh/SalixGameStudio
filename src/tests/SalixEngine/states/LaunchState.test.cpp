#define IMGUI_DEFINE_MATH_OPERATORS
#include <doctest.h>
#include <Salix/states/LaunchState.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <Salix/gui/DialogBox.h>
#include <Salix/management/ProjectManager.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Tests/SalixEngine/mocking/core/MockEngineInterface.h>
#include <Tests/SalixEngine/mocking/gui/MockIGui.h>
#include <Tests/SalixEngine/mocking/rendering/MockIRenderer.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace {

struct Batch7247ProjectRootGuard {
    std::filesystem::path previous = Salix::g_project_root_path;
    ~Batch7247ProjectRootGuard() { Salix::g_project_root_path = previous; }
};

struct Batch7247ImGuiFrame {
    ImGuiContext* previous = ImGui::GetCurrentContext();
    ImGuiContext* previous_engine = Salix::ImGuiInputManager::get_context_for_testing();
    ImGuiContext* owned = ImGui::CreateContext();

    Batch7247ImGuiFrame() {
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

    ~Batch7247ImGuiFrame() {
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

class Batch7247LaunchGui : public MockIGui {
public:
    Salix::DialogBox* create_dialog(
        std::string& key,
        std::string& title,
        Salix::DialogType type,
        bool overwrite) override {
        last_dialog_key = key;
        last_dialog_title = title;
        last_dialog_type = type;
        last_dialog_overwrite = overwrite;
        ++create_dialog_calls;

        auto dialog = std::make_unique<Salix::DialogBox>(key, title, type, overwrite);
        Salix::DialogBox* raw = dialog.get();
        dialogs[key] = std::move(dialog);
        return raw;
    }

    Salix::DialogBox* get_dialog(const std::string& key) override {
        auto it = dialogs.find(key);
        return it == dialogs.end() ? nullptr : it->second.get();
    }

    void display_dialogs() override {
        ++display_dialog_calls;
        if (!has_pending_result) {
            return;
        }

        auto it = dialogs.find(pending_key);
        const Salix::FileDialogResult result = pending_result;
        has_pending_result = false;
        pending_key.clear();

        if (it != dialogs.end()) {
            const auto& callback = it->second->get_callback();
            if (callback) {
                callback(result);
            }
        }
    }

    void queue_result(const std::string& key, const Salix::FileDialogResult& result) {
        pending_key = key;
        pending_result = result;
        has_pending_result = true;
    }

    std::unordered_map<std::string, std::unique_ptr<Salix::DialogBox>> dialogs;
    std::string pending_key;
    Salix::FileDialogResult pending_result{};
    bool has_pending_result = false;
};

Salix::InitContext batch7247_launch_context(
    MockEngineInterface& engine,
    Salix::IGui* gui = nullptr,
    Salix::ProjectManager* project_manager = nullptr,
    Salix::IRenderer* renderer = nullptr) {
    Salix::InitContext context{};
    context.engine = &engine;
    context.gui = gui;
    context.project_manager = project_manager;
    context.renderer = renderer;
    return context;
}

} // namespace

TEST_SUITE("Salix::states::LaunchStateBatch7247") {
    TEST_CASE("batch7247 launch state construction and destruction are safe") {
        Salix::LaunchState state;
        CHECK(true);
    }

    TEST_CASE("batch7247 launch on enter sets engine launch mode with null GUI") {
        MockEngineInterface engine;
        Salix::LaunchState state;
        const auto context = batch7247_launch_context(engine);
        state.on_enter(context);
        CHECK(engine.current_mode == Salix::EngineMode::Launch);
        CHECK(engine.set_mode_calls == 1);
    }

    TEST_CASE("batch7247 launch on enter accepts GUI context and sets mode once") {
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        const auto context = batch7247_launch_context(engine, &gui);
        state.on_enter(context);
        CHECK(engine.current_mode == Salix::EngineMode::Launch);
        CHECK(engine.set_mode_calls == 1);
    }

    TEST_CASE("batch7247 launch repeated on enter remains a valid mode transition") {
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        const auto context = batch7247_launch_context(engine, &gui);
        state.on_enter(context);
        state.on_enter(context);
        CHECK(engine.current_mode == Salix::EngineMode::Launch);
        CHECK(engine.set_mode_calls == 2);
    }

    TEST_CASE("batch7247 launch on exit is safe without renderer") {
        MockEngineInterface engine;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine));
        CHECK_NOTHROW(state.on_exit());
    }

    TEST_CASE("batch7247 launch on exit is safe with renderer in context") {
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        MockIRenderer renderer;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, nullptr, &renderer));
        CHECK_NOTHROW(state.on_exit());
    }

    TEST_CASE("batch7247 first GUI update creates exactly three launcher dialogs") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.create_dialog_calls == 3);
        CHECK(gui.dialogs.size() == 3);
    }

    TEST_CASE("batch7247 subsequent GUI update does not recreate launcher dialogs") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        frame.run([&] { state.update(1.0f); });
        CHECK(gui.create_dialog_calls == 3);
        CHECK(gui.display_dialog_calls == 2);
    }

    TEST_CASE("batch7247 launcher new project dialog has canonical save semantics") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        auto* dialog = gui.get_dialog("ChooseNewProjectLocation");
        REQUIRE(dialog != nullptr);
        CHECK(dialog->get_title() == "Create New Project");
        CHECK(dialog->get_type() == Salix::DialogType::File);
        CHECK(dialog->get_filters().find(".salixproj") != std::string::npos);
    }

    TEST_CASE("batch7247 launcher open project dialog has canonical load semantics") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        auto* dialog = gui.get_dialog("ChooseExistingProject");
        REQUIRE(dialog != nullptr);
        CHECK(dialog->get_title() == "Open An Existing Project");
        CHECK(dialog->get_filters().find(".salixproj") != std::string::npos);
    }

    TEST_CASE("batch7247 launcher run project dialog has canonical load semantics") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        auto* dialog = gui.get_dialog("ChooseExistingProjectToRun");
        REQUIRE(dialog != nullptr);
        CHECK(dialog->get_title() == "Run An Existing Project");
        CHECK(dialog->get_filters().find(".salixproj") != std::string::npos);
    }

    TEST_CASE("batch7247 canceled new project callback leaves state unchanged") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult result{};
        gui.queue_result("ChooseNewProjectLocation", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 accepted new project callback switches to editor") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        gui.queue_result("ChooseNewProjectLocation", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 1);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
    }

    TEST_CASE("batch7247 canceled open project callback leaves manager selection untouched") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        projects.set_project_to_load("Before7247.salixproj");
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        gui.queue_result("ChooseExistingProject", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 0);
        CHECK(projects.get_project_to_load() == "Before7247.salixproj");
    }

    TEST_CASE("batch7247 accepted open project callback stores exact selection and switches editor") {
        Batch7247ProjectRootGuard root_guard;
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        result.file_path_name = "C:/Batch7247/Game.salixproj";
        result.file_name = "Game.salixproj";
        result.folder_path = "C:/Batch7247";
        gui.queue_result("ChooseExistingProject", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(projects.get_project_to_load() == result.file_path_name);
        CHECK(projects.get_loaded_project_directory() == result.folder_path);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
    }

    TEST_CASE("batch7247 canceled run project callback leaves state unchanged") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult result{};
        gui.queue_result("ChooseExistingProjectToRun", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 accepted run project callback switches to game") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        gui.queue_result("ChooseExistingProjectToRun", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 1);
        CHECK(engine.current_state == Salix::AppStateType::Game);
    }

    TEST_CASE("batch7247 render with null renderer leaves GUI render untouched") {
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        state.render(nullptr);
        CHECK(gui.render_calls == 0);
        CHECK(gui.platform_window_calls == 0);
    }

    TEST_CASE("batch7247 render with mock renderer delegates both GUI render phases") {
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        MockIRenderer renderer;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        state.render(&renderer);
        CHECK(gui.render_calls == 1);
        CHECK(gui.platform_window_calls == 1);
    }

    TEST_CASE("batch7247 new project launcher button requests matching dialog") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "New Project", [&] { state.update(0.0f); }));
        CHECK(gui.show_dialog_calls == 1);
        CHECK(gui.last_dialog_key == "ChooseNewProjectLocation");
    }

    TEST_CASE("batch7247 open project launcher button requests matching dialog") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Open Project", [&] { state.update(0.0f); }));
        CHECK(gui.show_dialog_calls == 1);
        CHECK(gui.last_dialog_key == "ChooseExistingProject");
    }

    TEST_CASE("batch7247 run project launcher button requests matching dialog") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Run Project", [&] { state.update(0.0f); }));
        CHECK(gui.show_dialog_calls == 1);
        CHECK(gui.last_dialog_key == "ChooseExistingProjectToRun");
    }

    TEST_CASE("batch7247 options launcher button switches to options state") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Options", [&] { state.update(0.0f); }));
        CHECK(engine.current_state == Salix::AppStateType::Options);
        CHECK(engine.switch_state_calls == 1);
    }

    TEST_CASE("batch7247 quit launcher button stops engine") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Quit", [&] { state.update(0.0f); }));
        CHECK_FALSE(engine.running);
    }

    TEST_CASE("batch7247 idle launcher update preserves engine running state") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(-7247.0f); });
        CHECK(engine.running);
        CHECK(engine.switch_state_calls == 0);
    }
}


TEST_SUITE("Salix::states::LaunchStateBatch7247Additional") {
    TEST_CASE("batch7247 all three launcher dialogs install callbacks") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        REQUIRE(gui.get_dialog("ChooseNewProjectLocation") != nullptr);
        REQUIRE(gui.get_dialog("ChooseExistingProject") != nullptr);
        REQUIRE(gui.get_dialog("ChooseExistingProjectToRun") != nullptr);
        CHECK(static_cast<bool>(gui.get_dialog("ChooseNewProjectLocation")->get_callback()));
        CHECK(static_cast<bool>(gui.get_dialog("ChooseExistingProject")->get_callback()));
        CHECK(static_cast<bool>(gui.get_dialog("ChooseExistingProjectToRun")->get_callback()));
    }

    TEST_CASE("batch7247 new project dialog requests overwrite confirmation") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        auto* dialog = gui.get_dialog("ChooseNewProjectLocation");
        REQUIRE(dialog != nullptr);
        CHECK((dialog->get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) != 0);
    }

    TEST_CASE("batch7247 open project dialog does not request overwrite confirmation") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        auto* dialog = gui.get_dialog("ChooseExistingProject");
        REQUIRE(dialog != nullptr);
        CHECK((dialog->get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
    }

    TEST_CASE("batch7247 run project dialog does not request overwrite confirmation") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        auto* dialog = gui.get_dialog("ChooseExistingProjectToRun");
        REQUIRE(dialog != nullptr);
        CHECK((dialog->get_config().flags & ImGuiFileDialogFlags_ConfirmOverwrite) == 0);
    }

    TEST_CASE("batch7247 launcher dialogs retain default dot path") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.get_dialog("ChooseNewProjectLocation")->get_default_path() == ".");
        CHECK(gui.get_dialog("ChooseExistingProject")->get_default_path() == ".");
        CHECK(gui.get_dialog("ChooseExistingProjectToRun")->get_default_path() == ".");
    }

    TEST_CASE("batch7247 all launcher dialogs are file dialogs") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.get_dialog("ChooseNewProjectLocation")->get_type() == Salix::DialogType::File);
        CHECK(gui.get_dialog("ChooseExistingProject")->get_type() == Salix::DialogType::File);
        CHECK(gui.get_dialog("ChooseExistingProjectToRun")->get_type() == Salix::DialogType::File);
    }

    TEST_CASE("batch7247 one idle launcher update displays dialogs exactly once") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.display_dialog_calls == 1);
    }

    TEST_CASE("batch7247 idle launcher does not request any dialog to open") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        frame.run([&] { state.update(0.0f); });
        CHECK(gui.show_dialog_calls == 0);
    }

    TEST_CASE("batch7247 new project button request alone does not switch app state") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "New Project", [&] { state.update(0.0f); }));
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 open project button request alone does not switch app state") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Open Project", [&] { state.update(0.0f); }));
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 run project button request alone does not switch app state") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Run Project", [&] { state.update(0.0f); }));
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 options button transition keeps engine running") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Options", [&] { state.update(0.0f); }));
        CHECK(engine.running);
        CHECK(engine.current_state == Salix::AppStateType::Options);
    }

    TEST_CASE("batch7247 quit button stops engine without state switch") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Quit", [&] { state.update(0.0f); }));
        CHECK_FALSE(engine.running);
        CHECK(engine.switch_state_calls == 0);
    }

    TEST_CASE("batch7247 quit button leaves launch mode unchanged") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        REQUIRE(frame.activate_button("Welcome to Salix Game Studio!", "Quit", [&] { state.update(0.0f); }));
        CHECK(engine.current_mode == Salix::EngineMode::Launch);
    }

    TEST_CASE("batch7247 accepted new project transition keeps engine running") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        gui.queue_result("ChooseNewProjectLocation", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.running);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
    }

    TEST_CASE("batch7247 accepted open project transition keeps engine running") {
        Batch7247ProjectRootGuard root_guard;
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        result.file_path_name = "C:/Batch7247/Open.salixproj";
        result.folder_path = "C:/Batch7247";
        gui.queue_result("ChooseExistingProject", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.running);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
    }

    TEST_CASE("batch7247 accepted run project transition keeps engine running") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        gui.queue_result("ChooseExistingProjectToRun", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.running);
        CHECK(engine.current_state == Salix::AppStateType::Game);
    }

    TEST_CASE("batch7247 accepted open project preserves spaces in selection metadata") {
        Batch7247ProjectRootGuard root_guard;
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        result.file_path_name = "C:/Batch 7247/My Game.salixproj";
        result.folder_path = "C:/Batch 7247";
        gui.queue_result("ChooseExistingProject", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(projects.get_project_to_load() == result.file_path_name);
        CHECK(projects.get_loaded_project_directory() == result.folder_path);
    }

    TEST_CASE("batch7247 accepted open project replaces prior pending selection") {
        Batch7247ProjectRootGuard root_guard;
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        projects.set_project_to_load("Old7247.salixproj");
        projects.set_loaded_project_directory("Old7247");
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        result.file_path_name = "New7247.salixproj";
        result.folder_path = "NewRoot7247";
        gui.queue_result("ChooseExistingProject", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(projects.get_project_to_load() == "New7247.salixproj");
        CHECK(projects.get_loaded_project_directory() == "NewRoot7247");
    }

    TEST_CASE("batch7247 canceled open project preserves both prior selection fields") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        projects.set_project_to_load("Keep7247.salixproj");
        projects.set_loaded_project_directory("KeepRoot7247");
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        gui.queue_result("ChooseExistingProject", Salix::FileDialogResult{});
        frame.run([&] { state.update(0.0f); });
        CHECK(projects.get_project_to_load() == "Keep7247.salixproj");
        CHECK(projects.get_loaded_project_directory() == "KeepRoot7247");
    }

    TEST_CASE("batch7247 accepted new project callback does not mutate project manager selection") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        projects.set_project_to_load("KeepNew7247.salixproj");
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        gui.queue_result("ChooseNewProjectLocation", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(projects.get_project_to_load() == "KeepNew7247.salixproj");
    }

    TEST_CASE("batch7247 accepted run project callback does not mutate project manager selection") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::ProjectManager projects;
        projects.set_project_to_load("KeepRun7247.salixproj");
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui, &projects));
        Salix::FileDialogResult result{};
        result.is_ok = true;
        gui.queue_result("ChooseExistingProjectToRun", result);
        frame.run([&] { state.update(0.0f); });
        CHECK(projects.get_project_to_load() == "KeepRun7247.salixproj");
    }

    TEST_CASE("batch7247 canceled then accepted new project yields only accepted transition") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        gui.queue_result("ChooseNewProjectLocation", Salix::FileDialogResult{});
        frame.run([&] { state.update(0.0f); });
        Salix::FileDialogResult accepted{};
        accepted.is_ok = true;
        gui.queue_result("ChooseNewProjectLocation", accepted);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 1);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
    }

    TEST_CASE("batch7247 sequential accepted run and new callbacks each transition once") {
        Batch7247ImGuiFrame frame;
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        Salix::FileDialogResult accepted{};
        accepted.is_ok = true;
        gui.queue_result("ChooseExistingProjectToRun", accepted);
        frame.run([&] { state.update(0.0f); });
        gui.queue_result("ChooseNewProjectLocation", accepted);
        frame.run([&] { state.update(0.0f); });
        CHECK(engine.switch_state_calls == 2);
        CHECK(engine.current_state == Salix::AppStateType::Editor);
    }

    TEST_CASE("batch7247 repeated mock renderer renders delegate GUI phases each time") {
        MockEngineInterface engine;
        Batch7247LaunchGui gui;
        MockIRenderer renderer;
        Salix::LaunchState state;
        state.on_enter(batch7247_launch_context(engine, &gui));
        state.render(&renderer);
        state.render(&renderer);
        CHECK(gui.render_calls == 2);
        CHECK(gui.platform_window_calls == 2);
    }
}
