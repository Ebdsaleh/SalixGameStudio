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
