#include <doctest.h>
#include <Salix/scripting/ScriptLoader.h>
#include <Salix/core/InitContext.h>
#include <Salix/core/EngineMode.h>
#include <string>
#include <type_traits>
#include <utility>

TEST_SUITE("Salix::scripting::ScriptLoaderBatch1647") {
    TEST_CASE("script loader remains default constructible without loading a plugin") {
        CHECK((std::is_default_constructible_v<Salix::ScriptLoader>));
    }

    TEST_CASE("script loader remains destructible without explicit plugin ownership") {
        CHECK((std::is_destructible_v<Salix::ScriptLoader>));
    }

    TEST_CASE("script loader load operation retains boolean result contract") {
        using Return = decltype(Salix::ScriptLoader::load_script_library(
            std::declval<const std::string&>(),
            std::declval<Salix::InitContext&>()));
        CHECK((std::is_same_v<Return, bool>));
    }

    TEST_CASE("script loader shutdown operation retains void lifecycle contract") {
        using Return = decltype(std::declval<Salix::ScriptLoader&>().shutdown_plugin());
        CHECK((std::is_same_v<Return, void>));
    }

    TEST_CASE("script loader shutdown is safe before any plugin library was loaded") {
        Salix::ScriptLoader loader;
        loader.shutdown_plugin();
        CHECK(true);
    }

    TEST_CASE("script loader repeated shutdown remains safe with no loaded plugin") {
        Salix::ScriptLoader loader;
        loader.shutdown_plugin();
        loader.shutdown_plugin();
        loader.shutdown_plugin();
        CHECK(true);
    }

    TEST_CASE("script loader rejects empty library path") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("", context));
    }

    TEST_CASE("script loader rejects a uniquely missing relative DLL") {
        Salix::InitContext context{};
        const std::string path = "__salix_batch1647_missing_plugin_7f92a.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(path, context));
    }

    TEST_CASE("script loader failed load leaves supplied initialization context unchanged") {
        Salix::InitContext context{};
        context.dpi_scale = 2.25f;
        context.engine_mode = Salix::EngineMode::Debug;
        context.project_dll_path = "Before/Failure.dll";

        const auto before_path = context.project_dll_path;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__salix_batch1647_missing_directory__/missing_plugin.dll", context));

        CHECK(context.dpi_scale == doctest::Approx(2.25f));
        CHECK(context.engine_mode == Salix::EngineMode::Debug);
        CHECK(context.project_dll_path == before_path);
    }

    TEST_CASE("script loader failure keeps no plugin shutdown path safe for subsequent attempts") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__salix_batch1647_missing_plugin_first__.dll", context));
        loader.shutdown_plugin();
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__salix_batch1647_missing_plugin_second__.dll", context));
        loader.shutdown_plugin();
    }
}

TEST_SUITE("Salix::scripting::ScriptLoaderBatch2447") {
    TEST_CASE("batch2447 existing system DLL without create_plugin export is rejected") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("kernel32.dll", context));
    }

    TEST_CASE("batch2447 second existing system DLL without plugin export is rejected") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("user32.dll", context));
    }

    TEST_CASE("batch2447 missing create_plugin export leaves supplied context untouched") {
        Salix::InitContext context{};
        context.dpi_scale = 3.5f;
        context.engine_mode = Salix::EngineMode::Options;
        context.project_dll_path = "Before2447.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("kernel32.dll", context));
        CHECK(context.dpi_scale == doctest::Approx(3.5f));
        CHECK(context.engine_mode == Salix::EngineMode::Options);
        CHECK(context.project_dll_path == "Before2447.dll");
    }

    TEST_CASE("batch2447 shutdown remains safe after existing DLL lacks plugin export") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("kernel32.dll", context));
        CHECK_NOTHROW(loader.shutdown_plugin());
        CHECK_NOTHROW(loader.shutdown_plugin());
    }

    TEST_CASE("batch2447 missing path containing spaces is rejected without context mutation") {
        Salix::InitContext context{};
        context.project_dll_path = "Stable2447.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__batch2447 missing directory__/missing plugin.dll", context));
        CHECK(context.project_dll_path == "Stable2447.dll");
    }

    TEST_CASE("batch2447 missing library without extension is rejected") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch2447_missing_library_no_extension__", context));
    }

    TEST_CASE("batch2447 repeated missing-library failures keep shutdown path harmless") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch2447_missing_a__.dll", context));
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch2447_missing_b__.dll", context));
        CHECK_NOTHROW(loader.shutdown_plugin());
    }

    TEST_CASE("batch2447 failed loads preserve all selected initialization enum modes") {
        Salix::InitContext context{};
        context.engine_mode = Salix::EngineMode::Game;
        context.renderer_type = Salix::RendererType::OpenGL;
        context.gui_type = Salix::GuiType::ImGui;
        context.timer_type = Salix::TimerType::Chrono;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch2447_missing_modes__.dll", context));
        CHECK(context.engine_mode == Salix::EngineMode::Game);
        CHECK(context.renderer_type == Salix::RendererType::OpenGL);
        CHECK(context.gui_type == Salix::GuiType::ImGui);
        CHECK(context.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch2447 failed export lookup preserves nonowning service pointers") {
        Salix::InitContext context{};
        context.engine = reinterpret_cast<Salix::EngineInterface*>(static_cast<std::uintptr_t>(0x2447));
        auto* before = context.engine;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("kernel32.dll", context));
        CHECK(context.engine == before);
    }

    TEST_CASE("batch2447 independent loader objects can safely clean up after shared failed load state") {
        Salix::InitContext context{};
        Salix::ScriptLoader first;
        Salix::ScriptLoader second;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch2447_shared_failure__.dll", context));
        CHECK_NOTHROW(first.shutdown_plugin());
        CHECK_NOTHROW(second.shutdown_plugin());
    }
}

