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

TEST_SUITE("Salix::scripting::ScriptLoaderBatch5347") {
    TEST_CASE("batch5347 empty library path remains a safe failed load") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("", context));
    }

    TEST_CASE("batch5347 uniquely missing relative DLL remains a safe failed load") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_relative__.dll", context));
    }

    TEST_CASE("batch5347 missing path containing spaces remains a safe failed load") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__batch5347 missing directory__/missing plugin.dll", context));
    }

    TEST_CASE("batch5347 extensionless missing library remains a safe failed load") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_extensionless__", context));
    }

    TEST_CASE("batch5347 repeated distinct missing loads remain independent failures") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_a__.dll", context));
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_b__.dll", context));
    }

    TEST_CASE("batch5347 failed load preserves DPI and project DLL path") {
        Salix::InitContext context{};
        context.dpi_scale = 5.347f;
        context.project_dll_path = "Stable5347.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_context__.dll", context));
        CHECK(context.dpi_scale == doctest::Approx(5.347f));
        CHECK(context.project_dll_path == "Stable5347.dll");
    }

    TEST_CASE("batch5347 failed load preserves selected engine and backend modes") {
        Salix::InitContext context{};
        context.engine_mode = Salix::EngineMode::Debug;
        context.renderer_type = Salix::RendererType::OpenGL;
        context.gui_type = Salix::GuiType::None;
        context.timer_type = Salix::TimerType::Chrono;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_modes__.dll", context));
        CHECK(context.engine_mode == Salix::EngineMode::Debug);
        CHECK(context.renderer_type == Salix::RendererType::OpenGL);
        CHECK(context.gui_type == Salix::GuiType::None);
        CHECK(context.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch5347 shutdown remains harmless after failed load") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_missing_shutdown__.dll", context));
        CHECK_NOTHROW(loader.shutdown_plugin());
        CHECK_NOTHROW(loader.shutdown_plugin());
    }

    TEST_CASE("batch5347 independent loader objects can both shut down after shared failure") {
        Salix::InitContext context{};
        Salix::ScriptLoader first, second;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5347_shared_missing__.dll", context));
        CHECK_NOTHROW(first.shutdown_plugin());
        CHECK_NOTHROW(second.shutdown_plugin());
    }

    TEST_CASE("batch5347 repeated same missing path remains false across shutdown boundary") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        const std::string path = "__batch5347_repeat_missing__.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(path, context));
        loader.shutdown_plugin();
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(path, context));
        loader.shutdown_plugin();
    }
}

TEST_SUITE("Salix::scripting::ScriptLoaderBatch5847") {
    TEST_CASE("batch5847 empty path remains a safe failed load") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("", context));
    }

    TEST_CASE("batch5847 unique missing relative DLL remains false") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5847_missing_relative__.dll", context));
    }

    TEST_CASE("batch5847 missing nested DLL remains false") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__batch5847_missing__/nested/plugin5847.dll", context));
    }

    TEST_CASE("batch5847 missing path with spaces remains false") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__batch5847 missing directory__/missing plugin.dll", context));
    }

    TEST_CASE("batch5847 extensionless missing library remains false") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5847_missing_extensionless__", context));
    }

    TEST_CASE("batch5847 failed load preserves DPI and project path") {
        Salix::InitContext context{};
        context.dpi_scale = 5.847f;
        context.project_dll_path = "Stable5847.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5847_missing_context__.dll", context));
        CHECK(context.dpi_scale == doctest::Approx(5.847f));
        CHECK(context.project_dll_path == "Stable5847.dll");
    }

    TEST_CASE("batch5847 failed load preserves selected engine and backend modes") {
        Salix::InitContext context{};
        context.engine_mode = Salix::EngineMode::Game;
        context.renderer_type = Salix::RendererType::OpenGL;
        context.gui_type = Salix::GuiType::None;
        context.timer_type = Salix::TimerType::Chrono;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5847_missing_modes__.dll", context));
        CHECK(context.engine_mode == Salix::EngineMode::Game);
        CHECK(context.renderer_type == Salix::RendererType::OpenGL);
        CHECK(context.gui_type == Salix::GuiType::None);
        CHECK(context.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch5847 shutdown remains harmless after failed load") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5847_missing_shutdown__.dll", context));
        CHECK_NOTHROW(loader.shutdown_plugin());
        CHECK_NOTHROW(loader.shutdown_plugin());
    }

    TEST_CASE("batch5847 independent loader objects remain harmless after shared failure") {
        Salix::InitContext context{};
        Salix::ScriptLoader first, second;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch5847_shared_missing__.dll", context));
        CHECK_NOTHROW(first.shutdown_plugin());
        CHECK_NOTHROW(second.shutdown_plugin());
    }

    TEST_CASE("batch5847 repeated same missing path remains false across shutdown boundary") {
        Salix::InitContext context{};
        Salix::ScriptLoader loader;
        const std::string path = "__batch5847_repeat_missing__.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(path, context));
        loader.shutdown_plugin();
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(path, context));
        loader.shutdown_plugin();
    }
}

TEST_SUITE("Salix::scripting::ScriptLoaderBatch6247") {
    TEST_CASE("batch6247 whitespace only path remains a safe failed load") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("   ", context));
    }

    TEST_CASE("batch6247 missing parent traversal path remains false") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__batch6247_missing__/../still_missing6247.dll", context));
    }

    TEST_CASE("batch6247 missing path with mixed separators remains false") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library(
            "__batch6247_missing__/nested\\plugin6247.dll", context));
    }

    TEST_CASE("batch6247 repeated empty path remains false across calls") {
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("", context));
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("", context));
    }

    TEST_CASE("batch6247 failed load preserves zero DPI and empty project path") {
        Salix::InitContext context{};
        context.dpi_scale = 0.0f;
        context.project_dll_path.clear();
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_zero__.dll", context));
        CHECK(context.dpi_scale == 0.0f);
        CHECK(context.project_dll_path.empty());
    }

    TEST_CASE("batch6247 failed load preserves negative DPI and selected modes") {
        Salix::InitContext context{};
        context.dpi_scale = -6.247f;
        context.engine_mode = Salix::EngineMode::Editor;
        context.renderer_type = Salix::RendererType::None;
        context.gui_type = Salix::GuiType::None;
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_negative__.dll", context));
        CHECK(context.dpi_scale == doctest::Approx(-6.247f));
        CHECK(context.engine_mode == Salix::EngineMode::Editor);
        CHECK(context.renderer_type == Salix::RendererType::None);
        CHECK(context.gui_type == Salix::GuiType::None);
    }

    TEST_CASE("batch6247 two independent contexts preserve their own metadata after failures") {
        Salix::InitContext first{};
        Salix::InitContext second{};
        first.dpi_scale = 6.247f;
        first.project_dll_path = "First6247.dll";
        second.dpi_scale = 4.726f;
        second.project_dll_path = "Second6247.dll";
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_first__.dll", first));
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_second__.dll", second));
        CHECK(first.project_dll_path == "First6247.dll");
        CHECK(second.project_dll_path == "Second6247.dll");
        CHECK(first.dpi_scale == doctest::Approx(6.247f));
        CHECK(second.dpi_scale == doctest::Approx(4.726f));
    }

    TEST_CASE("batch6247 shutdown before and after failed load remains harmless") {
        Salix::ScriptLoader loader;
        Salix::InitContext context{};
        CHECK_NOTHROW(loader.shutdown_plugin());
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_shutdown__.dll", context));
        CHECK_NOTHROW(loader.shutdown_plugin());
    }

    TEST_CASE("batch6247 several loader shutdowns remain harmless around shared failure") {
        Salix::ScriptLoader first, second, third;
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_shared__.dll", context));
        CHECK_NOTHROW(first.shutdown_plugin());
        CHECK_NOTHROW(second.shutdown_plugin());
        CHECK_NOTHROW(third.shutdown_plugin());
    }

    TEST_CASE("batch6247 distinct missing paths remain false across shutdown boundary") {
        Salix::ScriptLoader loader;
        Salix::InitContext context{};
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_a__.dll", context));
        loader.shutdown_plugin();
        CHECK_FALSE(Salix::ScriptLoader::load_script_library("__batch6247_missing_b__.dll", context));
        loader.shutdown_plugin();
    }
}
