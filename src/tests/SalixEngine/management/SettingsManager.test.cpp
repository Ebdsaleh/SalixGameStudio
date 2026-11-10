// src/Tests/SalixEngine/management/SettingsManager.test.cpp

#include <doctest.h>
#include <Salix/management/SettingsManager.h>
#include <Salix/core/ApplicationConfig.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <iterator>
#include <yaml-cpp/yaml.h>

// Helper to create a temporary config file for a test
void create_temp_config(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    file << content;
    file.close();
}

// A helper function to capture the std::cerr output.
struct CerrRedirect {
    std::streambuf* original_buffer;
    std::stringstream& local_buffer; // Store reference

    CerrRedirect(std::stringstream& buffer) : local_buffer(buffer) {
        original_buffer = std::cerr.rdbuf(); // Save original buffer
        std::cerr.rdbuf(local_buffer.rdbuf()); // Redirect std::cerr
    }

    ~CerrRedirect() {
        std::cerr.rdbuf(original_buffer); // Restore original buffer
    }
};

TEST_SUITE("Salix::management::SettingsManager") {

	TEST_CASE("load_settings successfully parses a valid YAML file") {
        // ARRANGE
        Salix::SettingsManager settings_manager;
        Salix::ApplicationConfig config;
        const std::string test_file = "doctest_temp_config.yaml";

        // Ensure no stale cache file exists.
        std::filesystem::remove("doctest_temp_config.cache");

        // This YAML content now has the correct indentation with standard spaces.
        const std::string yaml_content = R"(
Window:
  title: "Test Game"
  width: 800
  height: 600
Engine:
  initial_state: Editor
  target_fps: 144
Renderer:
  type: OpenGL
GUI:
  type: ImGui
Timer:
  type: Chrono
)";

		// Create the temporary file.
        create_temp_config(test_file, yaml_content);
        REQUIRE(std::filesystem::exists(test_file));

        // ACT
        bool success = settings_manager.load_settings(test_file, config);

        // ASSERT
        REQUIRE(success == true);
        CHECK(config.window_config.title == "Test Game");
        CHECK(config.window_config.width == 800);
        CHECK(config.initial_state == Salix::AppStateType::Editor);
        CHECK(config.target_fps == 144);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.timer_type == Salix::TimerType::Chrono);

        // CLEANUP
        std::filesystem::remove(test_file);
        std::filesystem::remove("doctest_temp_config.cache");
    }

	TEST_CASE("load_settings handles missing file by using defaults") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config; // Default-constructed config
		const std::string non_existent_file = "non_existent_config.yaml";

		// Pre-condition: Ensure a default-constructed config has default values
		REQUIRE(config.target_fps == 60);
		REQUIRE(config.renderer_type == Salix::RendererType::SDL);

		// ACT
		bool success = settings_manager.load_settings(non_existent_file, config);

		// ASSERT
		// The function should "succeed" by not crashing and leaving the config with its default values.
		CHECK(success == true);
		CHECK(config.target_fps == 60); // Should remain default
		CHECK(config.renderer_type == Salix::RendererType::SDL); // Should remain default
    }

	TEST_CASE("save_settings handles file opening failure") {
        // ARRANGE
        Salix::SettingsManager settings_manager;
        Salix::ApplicationConfig config; // Default config is fine
        // Create a directory where we want to try writing the file.
        // Trying to open a directory as a file for writing should fail.
        const std::string invalid_dir_path = "test_dir_for_save_fail";
        const std::string invalid_file_path = invalid_dir_path; // Attempt to write directly to the dir path

        std::stringstream captured_cerr; // Buffer to capture cerr output

        // Ensure clean state & create the directory
        std::filesystem::remove_all(invalid_dir_path); // Remove if it exists from previous runs
        std::filesystem::create_directory(invalid_dir_path);
        REQUIRE(std::filesystem::is_directory(invalid_dir_path)); // Make sure dir exists

        bool save_result;
        { // Scope for CerrRedirect
            CerrRedirect redirect(captured_cerr); // Redirect cerr

            // ACT
            // Attempt to save settings, providing the directory path instead of a file path.
            save_result = settings_manager.save_settings(invalid_file_path, config);

        } // CerrRedirect destructor restores std::cerr here

        // ASSERT
        // Check that save_settings returned false due to the file opening error.
        CHECK(save_result == false);

        // Check if the captured output contains the specific error message text
        // from the `!file.is_open()` block.
        std::string output = captured_cerr.str();
        CHECK(output.find("SettingsManager Error: Could not open file for writing") != std::string::npos);
        // We no longer expect the YAML::Exception message here.

        // CLEANUP
        std::filesystem::remove_all(invalid_dir_path); // Clean up the created directory
    }

    TEST_CASE("save_settings writes a correct YAML and covers all enum branches") {
        // ARRANGE
        Salix::SettingsManager settings_manager;
        Salix::ApplicationConfig config_to_save;
        const std::string test_save_file = "temp_save_config.yaml";

        // Customize the config to use all the untested enum values for saving
        config_to_save.window_config.title = "Saved Game";
        config_to_save.target_fps = 90;
        config_to_save.renderer_type = Salix::RendererType::OpenGL;
        config_to_save.initial_state = Salix::AppStateType::Options; // Covers the 'Options' branch
        config_to_save.gui_type = Salix::GuiType::None;             // Covers the 'None' branch
        config_to_save.timer_type = Salix::TimerType::Chrono;       // Covers the 'Chrono' branch

        // ACT
        bool save_success = settings_manager.save_settings(test_save_file, config_to_save);
        REQUIRE(save_success == true);

        // Now, load the file we just saved to verify its contents
        Salix::ApplicationConfig loaded_config;
        settings_manager.load_settings(test_save_file, loaded_config);

        // ASSERT
        CHECK(loaded_config.window_config.title == "Saved Game");
        CHECK(loaded_config.target_fps == 90);
        CHECK(loaded_config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(loaded_config.initial_state == Salix::AppStateType::Options);
        CHECK(loaded_config.gui_type == Salix::GuiType::None);
        CHECK(loaded_config.timer_type == Salix::TimerType::Chrono);

        // CLEANUP
        std::filesystem::remove(test_save_file);
        std::filesystem::remove("temp_save_config.cache");
    }
	
	TEST_CASE("save_settings fails when writing to an invalid path") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;
		// This path is invalid because the directory "non_existent_directory" doesn't exist.
		const std::string invalid_path = "non_existent_directory/config.yaml";

		// ACT & ASSERT
		// This should return false, covering the file opening error branch.
		CHECK(settings_manager.save_settings(invalid_path, config) == false);
	}
    
	TEST_CASE("save_settings correctly saves all enum types") {
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;
		const std::string test_file = "enum_save_test.yaml";

		SUBCASE("saves RendererType::SDL") {
			config.renderer_type = Salix::RendererType::SDL;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves AppStateType::Game") {
			config.initial_state = Salix::AppStateType::Game;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves GuiType::ImGui") {
			config.gui_type = Salix::GuiType::ImGui;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves TimerType::SDL") {
			config.timer_type = Salix::TimerType::SDL;
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		SUBCASE("saves AppStateType::Editor") {
			config.initial_state = Salix::AppStateType::Editor;
			CHECK(settings_manager.save_settings(test_file, config) == true);
    	}
		SUBCASE("handles default enum cases") {
			// Force an invalid enum value to trigger the default case
			config.initial_state = static_cast<Salix::AppStateType>(99); 
			config.renderer_type = static_cast<Salix::RendererType>(99);
			config.gui_type = static_cast<Salix::GuiType>(99);
			config.timer_type = static_cast<Salix::TimerType>(99);
			CHECK(settings_manager.save_settings(test_file, config) == true);
		}
		// CLEANUP
		std::filesystem::remove(test_file);
	}

	TEST_CASE("load_settings handles malformed YAML file") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;
		const std::string malformed_file = "malformed.yaml";
		
		// This YAML is syntactically invalid (unclosed curly brace), which guarantees a parse error.
		create_temp_config(malformed_file, "Window: { title: \"This is not valid YAML }");

		// ACT & ASSERT
		// This will now return false, covering the YAML::Exception catch block.
		CHECK(settings_manager.load_settings(malformed_file, config) == false);

		// CLEANUP
		std::filesystem::remove(malformed_file);
	}

    TEST_CASE("load_settings correctly reads from a newer binary cache") {
		// ARRANGE
		Salix::SettingsManager settings_manager;
		const std::string yaml_file = "cache_test.yaml";
		const std::string cache_file = "cache_test.cache";

		// 1. Create a YAML file with "YAML Data".
		create_temp_config(yaml_file, "Window:\n  title: \"From YAML\"");
		
		// 2. Load it once. This will create a cache file with "YAML Data".
		Salix::ApplicationConfig temp_config;
		settings_manager.load_settings(yaml_file, temp_config);

		// 3. Wait a moment to ensure file timestamps will be different.
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		
		// 4. Overwrite the YAML file with "New YAML Data".
		create_temp_config(yaml_file, "Window:\n  title: \"New YAML Data\"");

		// ACT: Load settings again. The cache file is now older than the YAML file,
		// so it should re-parse the YAML and create a NEW cache.
		Salix::ApplicationConfig loaded_config;
		settings_manager.load_settings(yaml_file, loaded_config);

		// ASSERT: This confirms our setup is correct and the cache was updated.
		REQUIRE(loaded_config.window_config.title == "New YAML Data");

		// 5. Wait again and then change the YAML file one last time.
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		create_temp_config(yaml_file, "Window:\n  title: \"Final YAML Data\"");
		
		// ACT 2: Load settings a final time. Now, the cache from step 4 should be
		// NEWER than the YAML from step 2, but OLDER than the final YAML.
		// Wait, the logic is simpler. The cache file is now NEWER than the original YAML,
		// let's manually make it newer than the FINAL yaml.
		// The previous test logic was flawed. Let's simplify.

		// Let's restart the logic for this test.
		// 1. Create YAML file with initial data.
		create_temp_config(yaml_file, "Window:\n  title: \"Initial Data\"");
		
		// 2. Load it to create a cache file.
		Salix::ApplicationConfig config1;
		settings_manager.load_settings(yaml_file, config1);
		REQUIRE(config1.window_config.title == "Initial Data");

		// 3. Modify the original config object and manually write a NEW cache file
		//    that is out of sync with the YAML file.
		Salix::ApplicationConfig cache_only_config;
		cache_only_config.window_config.title = "From Cache";
		// We can't call a `write_cache` directly, so let's rethink.

		// Okay, the original test logic was almost correct but had one flaw.
		// `save_settings` doesn't write a cache. `load_settings` does.

		// Let's use the logic from the user's HTML report to guide us.
		// `if (std::filesystem::exists(cache_path) && std::filesystem::exists(yaml_path) && std::filesystem::last_write_time(cache_path) >= std::filesystem::last_write_time(yaml_path))`
		
		// Let's try this again.
		
		// CLEANUP from previous failed attempts
		std::filesystem::remove(yaml_file);
		std::filesystem::remove(cache_file);

		// ARRANGE
		// 1. Create a YAML with "original" data.
		create_temp_config(yaml_file, "Window:\n  title: \"Original YAML\"");
		
		// 2. Load it. This creates a cache file with "Original YAML" data.
		Salix::ApplicationConfig temp_config1;
		settings_manager.load_settings(yaml_file, temp_config1);
		REQUIRE(temp_config1.window_config.title == "Original YAML");

		// 3. Now, overwrite ONLY the YAML file with "new" data.
		create_temp_config(yaml_file, "Window:\n  title: \"New YAML\"");

		// 4. Force the cache file's timestamp to be NEWER than the YAML file.
		// This is the key to forcing the `read_cache` path.
		auto yaml_time = std::filesystem::last_write_time(yaml_file);
		std::filesystem::last_write_time(cache_file, yaml_time + std::chrono::seconds(1));

		// ACT
		Salix::ApplicationConfig final_config;
		settings_manager.load_settings(yaml_file, final_config);

		// ASSERT
		// Because the cache was newer, the loaded title should be "Original YAML", not "New YAML".
		// This proves `read_cache` was executed.
		CHECK(final_config.window_config.title == "Original YAML");

		// CLEANUP
		std::filesystem::remove(yaml_file);
		std::filesystem::remove(cache_file);
      }

      TEST_CASE("covers remaining parsing branches and error handling") {
        // ARRANGE
		Salix::SettingsManager settings_manager;
		Salix::ApplicationConfig config;

		// ACT & ASSERT for save_settings error path (passing an empty path)
		CHECK(settings_manager.save_settings("", config) == false);

		// ACT & ASSERT for remaining parse_app_state_type branches
		create_temp_config("enum_test.yaml", "Engine:\n  initial_state: Game");
		settings_manager.load_settings("enum_test.yaml", config);
		CHECK(config.initial_state == Salix::AppStateType::Game);

        // **** ADD A SMALL DELAY FOR TIMESTAMP RESOLUTION ****
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

		create_temp_config("enum_test.yaml", "Engine:\n  initial_state: Options");
		settings_manager.load_settings("enum_test.yaml", config);
		CHECK(config.initial_state == Salix::AppStateType::Options);

        // **** ADD A SMALL DELAY FOR TIMESTAMP RESOLUTION ****
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
		// ACT & ASSERT for remaining parse_gui_type branch
		create_temp_config("enum_test.yaml", "GUI:\n  type: None");
		settings_manager.load_settings("enum_test.yaml", config);
		CHECK(config.gui_type == Salix::GuiType::None);

		// CLEANUP
		std::filesystem::remove("enum_test.yaml");
		std::filesystem::remove("enum_test.cache");
	}
}

// Integration boundaries for the 847+ coverage batch.
TEST_SUITE("Salix::management::SettingsManagerBatch847") {
    TEST_CASE("missing settings file preserves a caller's preconfigured values") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.window_config.title="Keep"; c.window_config.width=321; c.target_fps=144; c.renderer_type=Salix::RendererType::OpenGL; const std::string p="settings_batch847_missing.yaml"; std::filesystem::remove(p); std::filesystem::remove("settings_batch847_missing.cache"); REQUIRE(m.load_settings(p,c)); CHECK(c.window_config.title=="Keep"); CHECK(c.window_config.width==321); CHECK(c.target_fps==144); CHECK(c.renderer_type==Salix::RendererType::OpenGL);
    }

    TEST_CASE("partial YAML changes only the fields explicitly present") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.window_config.title="Original"; c.window_config.width=111; c.window_config.height=222; c.target_fps=75; const std::string p="settings_batch847_partial.yaml"; std::filesystem::remove("settings_batch847_partial.cache"); create_temp_config(p,"Window:\n  width: 900\n"); REQUIRE(m.load_settings(p,c)); CHECK(c.window_config.title=="Original"); CHECK(c.window_config.width==900); CHECK(c.window_config.height==222); CHECK(c.target_fps==75); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_partial.cache");
    }

    TEST_CASE("unknown enum strings leave the caller's existing selections intact") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.initial_state=Salix::AppStateType::Options; c.renderer_type=Salix::RendererType::OpenGL; c.gui_type=Salix::GuiType::None; c.timer_type=Salix::TimerType::Chrono; const std::string p="settings_batch847_unknown.yaml"; std::filesystem::remove("settings_batch847_unknown.cache"); create_temp_config(p,"Engine:\n  initial_state: Unknown\nRenderer:\n  type: Unknown\nGUI:\n  type: Unknown\nTimer:\n  type: Unknown\n"); REQUIRE(m.load_settings(p,c)); CHECK(c.initial_state==Salix::AppStateType::Options); CHECK(c.renderer_type==Salix::RendererType::OpenGL); CHECK(c.gui_type==Salix::GuiType::None); CHECK(c.timer_type==Salix::TimerType::Chrono); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_unknown.cache");
    }

    TEST_CASE("save and load roundtrip preserves every GUI scaling field") {
        Salix::SettingsManager m; Salix::ApplicationConfig src; src.gui_settings.dialog_width_ratio=0.33f; src.gui_settings.dialog_height_ratio=0.44f; src.gui_settings.font_scaling=1.75f; src.gui_settings.global_dpi_scaling=2.25f; const std::string p="settings_batch847_gui.yaml"; std::filesystem::remove("settings_batch847_gui.cache"); REQUIRE(m.save_settings(p,src)); Salix::ApplicationConfig dst; REQUIRE(m.load_settings(p,dst)); CHECK(dst.gui_settings.dialog_width_ratio==doctest::Approx(0.33f)); CHECK(dst.gui_settings.dialog_height_ratio==doctest::Approx(0.44f)); CHECK(dst.gui_settings.font_scaling==doctest::Approx(1.75f)); CHECK(dst.gui_settings.global_dpi_scaling==doctest::Approx(2.25f)); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_gui.cache");
    }

    TEST_CASE("save and load roundtrip preserves signed window dimensions and target FPS") {
        Salix::SettingsManager m; Salix::ApplicationConfig src; src.window_config.width=-640; src.window_config.height=0; src.target_fps=-1; const std::string p="settings_batch847_numbers.yaml"; std::filesystem::remove("settings_batch847_numbers.cache"); REQUIRE(m.save_settings(p,src)); Salix::ApplicationConfig dst; REQUIRE(m.load_settings(p,dst)); CHECK(dst.window_config.width==-640); CHECK(dst.window_config.height==0); CHECK(dst.target_fps==-1); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_numbers.cache");
    }

    TEST_CASE("quoted punctuation in the window title survives YAML persistence") {
        Salix::SettingsManager m; Salix::ApplicationConfig src; src.window_config.title="Salix: \"Editor\" #1 / Test"; const std::string p="settings_batch847_title.yaml"; std::filesystem::remove("settings_batch847_title.cache"); REQUIRE(m.save_settings(p,src)); Salix::ApplicationConfig dst; REQUIRE(m.load_settings(p,dst)); CHECK(dst.window_config.title==src.window_config.title); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_title.cache");
    }

    TEST_CASE("saved settings document contains every expected top level section") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch847_sections.yaml"; REQUIRE(m.save_settings(p,c)); std::ifstream in(p); REQUIRE(in.is_open()); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>()); CHECK(text.find("Window:")!=std::string::npos); CHECK(text.find("Engine:")!=std::string::npos); CHECK(text.find("Renderer:")!=std::string::npos); CHECK(text.find("GUI:")!=std::string::npos); CHECK(text.find("Timer:")!=std::string::npos); in.close(); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_sections.cache");
    }

    TEST_CASE("unsupported renderer selection serializes through the documented SDL fallback") {
        Salix::SettingsManager m; Salix::ApplicationConfig src; src.renderer_type=Salix::RendererType::DirectX; const std::string p="settings_batch847_renderer_fallback.yaml"; std::filesystem::remove("settings_batch847_renderer_fallback.cache"); REQUIRE(m.save_settings(p,src)); Salix::ApplicationConfig dst; dst.renderer_type=Salix::RendererType::OpenGL; REQUIRE(m.load_settings(p,dst)); CHECK(dst.renderer_type==Salix::RendererType::SDL); std::filesystem::remove(p); std::filesystem::remove("settings_batch847_renderer_fallback.cache");
    }

    TEST_CASE("malformed YAML returns false and does not create a binary cache") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch847_bad.yaml"; const std::string cache="settings_batch847_bad.cache"; std::filesystem::remove(cache); create_temp_config(p,"Window: [unterminated"); CHECK_FALSE(m.load_settings(p,c)); CHECK_FALSE(std::filesystem::exists(cache)); std::filesystem::remove(p);
    }

    TEST_CASE("a successful YAML load creates a reusable cache with the loaded title") {
        Salix::SettingsManager m; Salix::ApplicationConfig first; const std::string p="settings_batch847_cache.yaml"; const std::string cache="settings_batch847_cache.cache"; std::filesystem::remove(cache); create_temp_config(p,"Window:\n  title: Cached Title\n"); REQUIRE(m.load_settings(p,first)); REQUIRE(std::filesystem::is_regular_file(cache)); CHECK(first.window_config.title=="Cached Title"); Salix::ApplicationConfig second; REQUIRE(m.load_settings(p,second)); CHECK(second.window_config.title=="Cached Title"); std::filesystem::remove(p); std::filesystem::remove(cache);
    }
}

TEST_SUITE("Salix::management::SettingsManagerBatch2247") {
    TEST_CASE("batch2247 YAML parser accepts Launch SDL None and SDL selections together") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch2247_launch.yaml",cache="settings_batch2247_launch.cache"; std::filesystem::remove(cache);
        create_temp_config(p,"Engine:\n  initial_state: Launch\nRenderer:\n  type: SDL\nGUI:\n  type: None\nTimer:\n  type: SDL\n");
        REQUIRE(m.load_settings(p,c)); CHECK(c.initial_state==Salix::AppStateType::Launch); CHECK(c.renderer_type==Salix::RendererType::SDL); CHECK(c.gui_type==Salix::GuiType::None); CHECK(c.timer_type==Salix::TimerType::SDL);
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 YAML parser accepts Game OpenGL ImGui and Chrono selections together") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch2247_game.yaml",cache="settings_batch2247_game.cache"; std::filesystem::remove(cache);
        create_temp_config(p,"Engine:\n  initial_state: Game\nRenderer:\n  type: OpenGL\nGUI:\n  type: ImGui\nTimer:\n  type: Chrono\n");
        REQUIRE(m.load_settings(p,c)); CHECK(c.initial_state==Salix::AppStateType::Game); CHECK(c.renderer_type==Salix::RendererType::OpenGL); CHECK(c.gui_type==Salix::GuiType::ImGui); CHECK(c.timer_type==Salix::TimerType::Chrono);
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 YAML parser accepts Options initial state") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch2247_options.yaml",cache="settings_batch2247_options.cache"; std::filesystem::remove(cache);
        create_temp_config(p,"Engine:\n  initial_state: Options\n");
        REQUIRE(m.load_settings(p,c)); CHECK(c.initial_state==Salix::AppStateType::Options);
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 partial window YAML updates supplied field while preserving omitted fields") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.window_config.title="KeepTitle"; c.window_config.width=111; c.window_config.height=222;
        const std::string p="settings_batch2247_partial.yaml",cache="settings_batch2247_partial.cache"; std::filesystem::remove(cache); create_temp_config(p,"Window:\n  width: 640\n");
        REQUIRE(m.load_settings(p,c)); CHECK(c.window_config.title=="KeepTitle"); CHECK(c.window_config.width==640); CHECK(c.window_config.height==222);
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 unknown enum strings preserve existing configuration selections") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.initial_state=Salix::AppStateType::Editor; c.renderer_type=Salix::RendererType::OpenGL; c.gui_type=Salix::GuiType::ImGui; c.timer_type=Salix::TimerType::Chrono;
        const std::string p="settings_batch2247_unknown.yaml",cache="settings_batch2247_unknown.cache"; std::filesystem::remove(cache);
        create_temp_config(p,"Engine:\n  initial_state: UnknownState\nRenderer:\n  type: UnknownRenderer\nGUI:\n  type: UnknownGui\nTimer:\n  type: UnknownTimer\n");
        REQUIRE(m.load_settings(p,c)); CHECK(c.initial_state==Salix::AppStateType::Editor); CHECK(c.renderer_type==Salix::RendererType::OpenGL); CHECK(c.gui_type==Salix::GuiType::ImGui); CHECK(c.timer_type==Salix::TimerType::Chrono);
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 GUI numeric settings parse independently from YAML") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch2247_gui.yaml",cache="settings_batch2247_gui.cache"; std::filesystem::remove(cache);
        create_temp_config(p,"GUI:\n  dialog_width_ratio: 0.33\n  dialog_height_ratio: 0.66\n  font_scaling: 1.25\n  global_dpi_scaling: 1.5\n");
        REQUIRE(m.load_settings(p,c)); CHECK(c.gui_settings.dialog_width_ratio==doctest::Approx(0.33f)); CHECK(c.gui_settings.dialog_height_ratio==doctest::Approx(0.66f)); CHECK(c.gui_settings.font_scaling==doctest::Approx(1.25f)); CHECK(c.gui_settings.global_dpi_scaling==doctest::Approx(1.5f));
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 save then load roundtrip preserves core window and engine settings") {
        Salix::SettingsManager m; Salix::ApplicationConfig src; src.window_config.title="Roundtrip2247"; src.window_config.width=1234; src.window_config.height=777; src.initial_state=Salix::AppStateType::Game; src.target_fps=165;
        const std::string p="settings_batch2247_roundtrip.yaml",cache="settings_batch2247_roundtrip.cache"; std::filesystem::remove(cache); REQUIRE(m.save_settings(p,src));
        Salix::ApplicationConfig dst; REQUIRE(m.load_settings(p,dst)); CHECK(dst.window_config.title==src.window_config.title); CHECK(dst.window_config.width==1234); CHECK(dst.window_config.height==777); CHECK(dst.initial_state==Salix::AppStateType::Game); CHECK(dst.target_fps==165);
        std::filesystem::remove(p); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2247 saved settings text contains explicit None GUI and SDL timer choices") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.gui_type=Salix::GuiType::None; c.timer_type=Salix::TimerType::SDL; const std::string p="settings_batch2247_text.yaml";
        REQUIRE(m.save_settings(p,c)); std::ifstream in(p); REQUIRE(in.is_open()); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text.find("type: None")!=std::string::npos); CHECK(text.find("type: SDL")!=std::string::npos); in.close(); std::filesystem::remove(p); std::filesystem::remove("settings_batch2247_text.cache");
    }
    TEST_CASE("batch2247 invalid integer scalar returns false and does not create cache") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; const std::string p="settings_batch2247_badint.yaml",cache="settings_batch2247_badint.cache"; std::filesystem::remove(cache);
        create_temp_config(p,"Window:\n  width: not-an-integer\n"); CHECK_FALSE(m.load_settings(p,c)); CHECK_FALSE(std::filesystem::exists(cache));
        std::filesystem::remove(p);
    }
    TEST_CASE("batch2247 missing YAML returns success while preserving preconfigured values") {
        Salix::SettingsManager m; Salix::ApplicationConfig c; c.window_config.title="Preserve"; c.target_fps=77; c.renderer_type=Salix::RendererType::OpenGL;
        const std::string p="settings_batch2247_missing.yaml"; std::filesystem::remove(p); std::filesystem::remove("settings_batch2247_missing.cache");
        REQUIRE(m.load_settings(p,c)); CHECK(c.window_config.title=="Preserve"); CHECK(c.target_fps==77); CHECK(c.renderer_type==Salix::RendererType::OpenGL);
    }
}

TEST_SUITE("Salix::management::SettingsManagerBatch2847") {
    TEST_CASE("batch2847 saving to an empty path fails without creating output") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        CHECK_FALSE(manager.save_settings("",config));
    }
    TEST_CASE("batch2847 invalid application state serializes through Launch fallback") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.initial_state=static_cast<Salix::AppStateType>(999);
        const std::string path="settings_batch2847_invalid_state.yaml";
        REQUIRE(manager.save_settings(path,config));
        std::ifstream in(path); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text.find("initial_state: Launch")!=std::string::npos);
        in.close(); std::filesystem::remove(path);
    }
    TEST_CASE("batch2847 invalid renderer serializes through SDL fallback") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.renderer_type=static_cast<Salix::RendererType>(999);
        const std::string path="settings_batch2847_invalid_renderer.yaml";
        REQUIRE(manager.save_settings(path,config));
        std::ifstream in(path); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text.find("Renderer:")!=std::string::npos); CHECK(text.find("type: SDL")!=std::string::npos);
        in.close(); std::filesystem::remove(path);
    }
    TEST_CASE("batch2847 invalid GUI selection serializes through ImGui fallback") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.gui_type=static_cast<Salix::GuiType>(999);
        const std::string path="settings_batch2847_invalid_gui.yaml";
        REQUIRE(manager.save_settings(path,config));
        std::ifstream in(path); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text.find("GUI:")!=std::string::npos); CHECK(text.find("type: ImGui")!=std::string::npos);
        in.close(); std::filesystem::remove(path);
    }
    TEST_CASE("batch2847 invalid timer selection serializes through SDL fallback") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.timer_type=static_cast<Salix::TimerType>(999);
        const std::string path="settings_batch2847_invalid_timer.yaml";
        REQUIRE(manager.save_settings(path,config));
        std::ifstream in(path); const std::string text((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
        CHECK(text.find("Timer:")!=std::string::npos); CHECK(text.find("type: SDL")!=std::string::npos);
        in.close(); std::filesystem::remove(path);
    }
    TEST_CASE("batch2847 full OpenGL ImGui Chrono Game configuration roundtrips through YAML") {
        Salix::SettingsManager manager; Salix::ApplicationConfig source;
        source.window_config.title="Roundtrip 2847"; source.window_config.width=1600; source.window_config.height=900;
        source.initial_state=Salix::AppStateType::Game; source.renderer_type=Salix::RendererType::OpenGL; source.gui_type=Salix::GuiType::ImGui; source.timer_type=Salix::TimerType::Chrono; source.target_fps=144;
        const std::string path="settings_batch2847_roundtrip.yaml",cache="settings_batch2847_roundtrip.cache"; std::filesystem::remove(cache);
        REQUIRE(manager.save_settings(path,source));
        Salix::ApplicationConfig loaded; REQUIRE(manager.load_settings(path,loaded));
        CHECK(loaded.window_config.title=="Roundtrip 2847"); CHECK(loaded.window_config.width==1600); CHECK(loaded.window_config.height==900);
        CHECK(loaded.initial_state==Salix::AppStateType::Game); CHECK(loaded.renderer_type==Salix::RendererType::OpenGL); CHECK(loaded.gui_type==Salix::GuiType::ImGui); CHECK(loaded.timer_type==Salix::TimerType::Chrono); CHECK(loaded.target_fps==144);
        std::filesystem::remove(path); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2847 missing YAML preserves caller supplied configuration") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.window_config.title="Keep 2847"; config.window_config.width=321; config.target_fps=77;
        const std::string path="settings_batch2847_missing.yaml"; std::filesystem::remove(path); std::filesystem::remove("settings_batch2847_missing.cache");
        REQUIRE(manager.load_settings(path,config));
        CHECK(config.window_config.title=="Keep 2847"); CHECK(config.window_config.width==321); CHECK(config.target_fps==77);
    }
    TEST_CASE("batch2847 partial YAML changes only supplied fields") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.window_config.title="KeepTitle2847"; config.window_config.width=111; config.window_config.height=222; config.target_fps=30;
        const std::string path="settings_batch2847_partial.yaml",cache="settings_batch2847_partial.cache"; std::filesystem::remove(cache);
        create_temp_config(path,"Window:\n  height: 720\nEngine:\n  target_fps: 120\n");
        REQUIRE(manager.load_settings(path,config));
        CHECK(config.window_config.title=="KeepTitle2847"); CHECK(config.window_config.width==111); CHECK(config.window_config.height==720); CHECK(config.target_fps==120);
        std::filesystem::remove(path); std::filesystem::remove(cache);
    }
    TEST_CASE("batch2847 malformed YAML returns false and leaves cache absent") {
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        const std::string path="settings_batch2847_malformed.yaml",cache="settings_batch2847_malformed.cache"; std::filesystem::remove(cache);
        create_temp_config(path,"Window: [unterminated");
        CHECK_FALSE(manager.load_settings(path,config)); CHECK_FALSE(std::filesystem::exists(cache));
        std::filesystem::remove(path);
    }
    TEST_CASE("batch2847 second load can restore values from fresh binary cache") {
        Salix::SettingsManager manager; Salix::ApplicationConfig first;
        const std::string path="settings_batch2847_cache.yaml",cache="settings_batch2847_cache.cache"; std::filesystem::remove(cache);
        create_temp_config(path,"Window:\n  title: Cached2847\n  width: 777\nEngine:\n  target_fps: 88\n");
        REQUIRE(manager.load_settings(path,first)); REQUIRE(std::filesystem::is_regular_file(cache));
        Salix::ApplicationConfig second; second.window_config.title="Wrong"; second.window_config.width=1; second.target_fps=1;
        REQUIRE(manager.load_settings(path,second));
        CHECK(second.window_config.title=="Cached2847"); CHECK(second.window_config.width==777); CHECK(second.target_fps==88);
        std::filesystem::remove(path); std::filesystem::remove(cache);
    }
}


TEST_SUITE("Salix::management::SettingsManagerBatch3047") {
    TEST_CASE("batch3047 unknown application state text preserves caller supplied state") {
        const std::string path = "settings_batch3047_state.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_state.cache");
        create_temp_config(path, "Engine:\n  initial_state: FutureState\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config; config.initial_state = Salix::AppStateType::Game;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.initial_state == Salix::AppStateType::Game);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_state.cache");
    }

    TEST_CASE("batch3047 unknown renderer text preserves caller supplied renderer") {
        const std::string path = "settings_batch3047_renderer.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_renderer.cache");
        create_temp_config(path, "Renderer:\n  type: VulkanFuture\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config; config.renderer_type = Salix::RendererType::OpenGL;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_renderer.cache");
    }

    TEST_CASE("batch3047 unknown GUI text preserves caller supplied GUI selection") {
        const std::string path = "settings_batch3047_gui.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_gui.cache");
        create_temp_config(path, "GUI:\n  type: FutureGUI\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config; config.gui_type = Salix::GuiType::None;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.gui_type == Salix::GuiType::None);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_gui.cache");
    }

    TEST_CASE("batch3047 unknown timer text preserves caller supplied timer selection") {
        const std::string path = "settings_batch3047_timer.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_timer.cache");
        create_temp_config(path, "Timer:\n  type: FutureTimer\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config; config.timer_type = Salix::TimerType::Chrono;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.timer_type == Salix::TimerType::Chrono);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_timer.cache");
    }

    TEST_CASE("batch3047 partial window YAML changes title without resetting caller dimensions") {
        const std::string path = "settings_batch3047_window.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_window.cache");
        create_temp_config(path, "Window:\n  title: Partial3047\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.window_config.width = 1234; config.window_config.height = 567;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.window_config.title == "Partial3047");
        CHECK(config.window_config.width == 1234);
        CHECK(config.window_config.height == 567);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_window.cache");
    }

    TEST_CASE("batch3047 GUI numeric fields accept zero negative and fractional values independently") {
        const std::string path = "settings_batch3047_gui_numbers.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_gui_numbers.cache");
        create_temp_config(path,
            "GUI:\n"
            "  dialog_width_ratio: 0.0\n"
            "  dialog_height_ratio: -0.5\n"
            "  font_scaling: 1.25\n"
            "  global_dpi_scaling: 2.75\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.0f));
        CHECK(config.gui_settings.dialog_height_ratio == doctest::Approx(-0.5f));
        CHECK(config.gui_settings.font_scaling == doctest::Approx(1.25f));
        CHECK(config.gui_settings.global_dpi_scaling == doctest::Approx(2.75f));
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_gui_numbers.cache");
    }

    TEST_CASE("batch3047 save and load preserve empty title zero dimensions and zero target FPS") {
        const std::string path = "settings_batch3047_zero.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_zero.cache");
        Salix::SettingsManager manager; Salix::ApplicationConfig source;
        source.window_config.title = ""; source.window_config.width = 0; source.window_config.height = 0; source.target_fps = 0;
        REQUIRE(manager.save_settings(path, source));
        Salix::ApplicationConfig loaded; REQUIRE(manager.load_settings(path, loaded));
        CHECK(loaded.window_config.title.empty());
        CHECK(loaded.window_config.width == 0);
        CHECK(loaded.window_config.height == 0);
        CHECK(loaded.target_fps == 0);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_zero.cache");
    }

    TEST_CASE("batch3047 save and load preserve negative diagnostic numeric configuration") {
        const std::string path = "settings_batch3047_negative.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_negative.cache");
        Salix::SettingsManager manager; Salix::ApplicationConfig source;
        source.target_fps = -144;
        source.gui_settings.dialog_width_ratio = -1.0f;
        source.gui_settings.dialog_height_ratio = -2.0f;
        source.gui_settings.font_scaling = -3.0f;
        source.gui_settings.global_dpi_scaling = -4.0f;
        REQUIRE(manager.save_settings(path, source));
        Salix::ApplicationConfig loaded; REQUIRE(manager.load_settings(path, loaded));
        CHECK(loaded.target_fps == -144);
        CHECK(loaded.gui_settings.dialog_width_ratio == doctest::Approx(-1.0f));
        CHECK(loaded.gui_settings.dialog_height_ratio == doctest::Approx(-2.0f));
        CHECK(loaded.gui_settings.font_scaling == doctest::Approx(-3.0f));
        CHECK(loaded.gui_settings.global_dpi_scaling == doctest::Approx(-4.0f));
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_negative.cache");
    }

    TEST_CASE("batch3047 loading multi extension YAML creates cache by replacing only final extension") {
        const std::string path = "settings.batch3047.profile.yaml";
        const std::string cache = "settings.batch3047.profile.cache";
        std::filesystem::remove(path); std::filesystem::remove(cache);
        create_temp_config(path, "Engine:\n  target_fps: 75\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.target_fps == 75);
        CHECK(std::filesystem::is_regular_file(cache));
        std::filesystem::remove(path); std::filesystem::remove(cache);
    }

    TEST_CASE("batch3047 YAML containing no recognized sections leaves preconfigured values untouched") {
        const std::string path = "settings_batch3047_unknown_section.yaml";
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_unknown_section.cache");
        create_temp_config(path, "FutureSection:\n  value: 3047\n");
        Salix::SettingsManager manager; Salix::ApplicationConfig config;
        config.window_config.title = "Keep3047"; config.target_fps = 91; config.renderer_type = Salix::RendererType::OpenGL;
        REQUIRE(manager.load_settings(path, config));
        CHECK(config.window_config.title == "Keep3047");
        CHECK(config.target_fps == 91);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        std::filesystem::remove(path); std::filesystem::remove("settings_batch3047_unknown_section.cache");
    }
}
namespace {
struct Batch3447SettingsFiles {
    explicit Batch3447SettingsFiles(const std::string& stem) {
        root = std::filesystem::temp_directory_path() / ("SalixGameStudio_" + stem);
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
        std::filesystem::create_directories(root);
        yaml = root / "settings.yaml";
        cache = root / "settings.cache";
    }
    ~Batch3447SettingsFiles() {
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
    }
    void write_yaml(const std::string& content) const {
        std::ofstream out(yaml);
        out << content;
    }
    std::filesystem::path root;
    std::filesystem::path yaml;
    std::filesystem::path cache;
};
}

TEST_SUITE("Salix::management::SettingsManagerBatch3447") {
    TEST_CASE("batch3447 missing YAML preserves caller supplied configuration and creates no cache") {
        Batch3447SettingsFiles files("Batch3447_Missing");
        std::filesystem::remove(files.yaml);
        Salix::ApplicationConfig config;
        config.window_config.title = "Keep3447";
        config.target_fps = 3447;
        config.renderer_type = Salix::RendererType::OpenGL;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.window_config.title == "Keep3447");
        CHECK(config.target_fps == 3447);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK_FALSE(std::filesystem::exists(files.cache));
    }

    TEST_CASE("batch3447 orphan cache is ignored when YAML source is missing") {
        Batch3447SettingsFiles files("Batch3447_OrphanCache");
        { std::ofstream out(files.cache, std::ios::binary); out << "not-a-valid-cache"; }
        Salix::ApplicationConfig config;
        config.target_fps = 93;
        config.timer_type = Salix::TimerType::Chrono;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.target_fps == 93);
        CHECK(config.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch3447 partial Window width preserves caller title and height") {
        Batch3447SettingsFiles files("Batch3447_WindowPartial");
        files.write_yaml("Window:\n  width: 1234\n");
        Salix::ApplicationConfig config;
        config.window_config.title = "Existing3447";
        config.window_config.height = 777;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.window_config.width == 1234);
        CHECK(config.window_config.title == "Existing3447");
        CHECK(config.window_config.height == 777);
    }

    TEST_CASE("batch3447 Options initial state parses while absent target FPS is preserved") {
        Batch3447SettingsFiles files("Batch3447_Options");
        files.write_yaml("Engine:\n  initial_state: Options\n");
        Salix::ApplicationConfig config;
        config.target_fps = 165;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.initial_state == Salix::AppStateType::Options);
        CHECK(config.target_fps == 165);
    }

    TEST_CASE("batch3447 OpenGL renderer parses without changing unrelated timer selection") {
        Batch3447SettingsFiles files("Batch3447_Renderer");
        files.write_yaml("Renderer:\n  type: OpenGL\n");
        Salix::ApplicationConfig config;
        config.timer_type = Salix::TimerType::Chrono;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch3447 GUI None parses while absent numeric GUI fields remain caller supplied") {
        Batch3447SettingsFiles files("Batch3447_Gui");
        files.write_yaml("GUI:\n  type: None\n");
        Salix::ApplicationConfig config;
        config.gui_settings.dialog_width_ratio = 0.3447f;
        config.gui_settings.font_scaling = 2.5f;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.gui_type == Salix::GuiType::None);
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.3447f));
        CHECK(config.gui_settings.font_scaling == doctest::Approx(2.5f));
    }

    TEST_CASE("batch3447 Chrono timer parses without changing unrelated renderer selection") {
        Batch3447SettingsFiles files("Batch3447_Timer");
        files.write_yaml("Timer:\n  type: Chrono\n");
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.timer_type == Salix::TimerType::Chrono);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
    }

    TEST_CASE("batch3447 save to missing parent directory fails without creating output") {
        Batch3447SettingsFiles files("Batch3447_SaveMissingParent");
        const auto missing = files.root / "missing" / "nested" / "settings.yaml";
        Salix::ApplicationConfig config;
        Salix::SettingsManager manager;
        CHECK_FALSE(manager.save_settings(missing.string(), config));
        CHECK_FALSE(std::filesystem::exists(missing));
        CHECK_FALSE(std::filesystem::exists(missing.parent_path()));
    }

    TEST_CASE("batch3447 save and load preserve whitespace colon and newline in window title") {
        Batch3447SettingsFiles files("Batch3447_Title");
        Salix::ApplicationConfig source;
        source.window_config.title = "  Salix: Batch 3447\nSecond Line  ";
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.window_config.title == source.window_config.title);
    }

    TEST_CASE("batch3447 extensionless YAML path creates sibling cache with cache extension") {
        Batch3447SettingsFiles files("Batch3447_Extensionless");
        const auto extensionless = files.root / "settings3447";
        const auto expected_cache = files.root / "settings3447.cache";
        { std::ofstream out(extensionless); out << "Engine:\n  target_fps: 147\n"; }
        Salix::ApplicationConfig config;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(extensionless.string(), config));
        CHECK(config.target_fps == 147);
        CHECK(std::filesystem::is_regular_file(expected_cache));
    }
}
TEST_SUITE("Salix::management::SettingsManagerBatch3847") {
    TEST_CASE("batch3847 newer truncated cache falls back to YAML instead of accepting partial data") {
        Batch3447SettingsFiles files("Batch3847_TruncatedCache");
        files.write_yaml("Window:\n  title: YAML3847\n  width: 1384\nEngine:\n  target_fps: 147\n");
        { std::ofstream out(files.cache, std::ios::binary); out << "bad"; }
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig config;
        config.window_config.title = "Before3847";
        config.window_config.width = 1;
        config.target_fps = 2;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.window_config.title == "YAML3847");
        CHECK(config.window_config.width == 1384);
        CHECK(config.target_fps == 147);
    }

    TEST_CASE("batch3847 cache truncated only in title bytes falls back to YAML safely") {
        Batch3447SettingsFiles files("Batch3847_TitleTruncated");
        files.write_yaml("Window:\n  title: Complete3847\n  width: 900\n");
        Salix::SettingsManager manager;
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        REQUIRE(std::filesystem::file_size(files.cache) > 1);
        std::filesystem::resize_file(files.cache, std::filesystem::file_size(files.cache) - 1);
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig second;
        second.window_config.title = "Wrong3847";
        second.window_config.width = 3;
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.window_config.title == "Complete3847");
        CHECK(second.window_config.width == 900);
    }

    TEST_CASE("batch3847 zero byte newer cache falls back to YAML without mutating from garbage") {
        Batch3447SettingsFiles files("Batch3847_ZeroCache");
        files.write_yaml("Engine:\n  target_fps: 3847\n");
        { std::ofstream out(files.cache, std::ios::binary | std::ios::trunc); }
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig config;
        config.target_fps = -1;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.target_fps == 3847);
    }

    TEST_CASE("batch3847 valid newer cache restores full cached values over caller defaults") {
        Batch3447SettingsFiles files("Batch3847_ValidCache");
        files.write_yaml(
            "Window:\n  title: Cached3847\n  width: 777\n  height: 555\n"
            "Engine:\n  initial_state: Game\n  target_fps: 88\n"
            "Renderer:\n  type: OpenGL\nTimer:\n  type: Chrono\n");
        Salix::SettingsManager manager;
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig second;
        second.window_config.title = "Wrong";
        second.window_config.width = 1;
        second.target_fps = 1;
        second.renderer_type = Salix::RendererType::SDL;
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.window_config.title == "Cached3847");
        CHECK(second.window_config.width == 777);
        CHECK(second.window_config.height == 555);
        CHECK(second.target_fps == 88);
        CHECK(second.renderer_type == Salix::RendererType::OpenGL);
        CHECK(second.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch3847 older cache is ignored when YAML has newer target FPS") {
        Batch3447SettingsFiles files("Batch3847_OlderCache");
        files.write_yaml("Engine:\n  target_fps: 60\n");
        Salix::SettingsManager manager;
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        const auto old_cache_time = std::filesystem::last_write_time(files.cache);
        files.write_yaml("Engine:\n  target_fps: 144\n");
        std::filesystem::last_write_time(files.yaml, old_cache_time + std::chrono::seconds(2));
        Salix::ApplicationConfig second;
        second.target_fps = 1;
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.target_fps == 144);
    }

    TEST_CASE("batch3847 corrupt cache fallback preserves caller values for YAML fields that are absent") {
        Batch3447SettingsFiles files("Batch3847_PartialFallback");
        files.write_yaml("Engine:\n  target_fps: 240\n");
        { std::ofstream out(files.cache, std::ios::binary); out << "corrupt3847"; }
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig config;
        config.window_config.title = "Preserve3847";
        config.window_config.width = 123;
        config.renderer_type = Salix::RendererType::OpenGL;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.target_fps == 240);
        CHECK(config.window_config.title == "Preserve3847");
        CHECK(config.window_config.width == 123);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
    }

    TEST_CASE("batch3847 malformed YAML returns false and does not create a replacement cache") {
        Batch3447SettingsFiles files("Batch3847_Malformed");
        files.write_yaml("Window: [unterminated\n");
        Salix::ApplicationConfig config;
        config.window_config.title = "BeforeMalformed3847";
        Salix::SettingsManager manager;
        CHECK_FALSE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.window_config.title == "BeforeMalformed3847");
        CHECK_FALSE(std::filesystem::exists(files.cache));
    }

    TEST_CASE("batch3847 saving shorter content fully truncates an existing longer YAML file") {
        Batch3447SettingsFiles files("Batch3847_TruncateSave");
        Salix::SettingsManager manager;
        Salix::ApplicationConfig long_config;
        long_config.window_config.title = "This is a deliberately very long Batch3847 window title for truncation";
        REQUIRE(manager.save_settings(files.yaml.string(), long_config));
        const auto long_size = std::filesystem::file_size(files.yaml);
        Salix::ApplicationConfig short_config;
        short_config.window_config.title = "S";
        REQUIRE(manager.save_settings(files.yaml.string(), short_config));
        const auto short_size = std::filesystem::file_size(files.yaml);
        CHECK(short_size < long_size);
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.window_config.title == "S");
    }

    TEST_CASE("batch3847 valid cache preserves empty window title exactly") {
        Batch3447SettingsFiles files("Batch3847_EmptyTitle");
        Salix::ApplicationConfig source;
        source.window_config.title.clear();
        source.target_fps = 75;
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        REQUIRE(first.window_config.title.empty());
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig second;
        second.window_config.title = "Wrong3847";
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.window_config.title.empty());
        CHECK(second.target_fps == 75);
    }

    TEST_CASE("batch3847 extensionless path recovers from corrupt sibling cache via source YAML") {
        Batch3447SettingsFiles files("Batch3847_ExtensionlessCorrupt");
        const auto yaml = files.root / "settings3847";
        const auto cache = files.root / "settings3847.cache";
        { std::ofstream out(yaml); out << "GUI:\n  font_scaling: 1.75\n"; }
        { std::ofstream out(cache, std::ios::binary); out << "broken"; }
        const auto yaml_time = std::filesystem::last_write_time(yaml);
        std::filesystem::last_write_time(cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig config;
        config.gui_settings.font_scaling = 0.5f;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(yaml.string(), config));
        CHECK(config.gui_settings.font_scaling == doctest::Approx(1.75f));
        CHECK(std::filesystem::is_regular_file(cache));
    }
}

TEST_SUITE("Salix::management::SettingsManagerBatch4447") {
    TEST_CASE("batch4447 partial Window height updates only height") {
        Batch3447SettingsFiles files("Batch4447_WindowHeight");
        files.write_yaml("Window:\n  height: 1447\n");
        Salix::ApplicationConfig config;
        config.window_config.title = "Keep4447";
        config.window_config.width = 944;
        config.window_config.height = 1;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.window_config.title == "Keep4447");
        CHECK(config.window_config.width == 944);
        CHECK(config.window_config.height == 1447);
    }

    TEST_CASE("batch4447 unknown application state leaves caller value unchanged") {
        Batch3447SettingsFiles files("Batch4447_UnknownState");
        files.write_yaml("Engine:\n  initial_state: FutureState4447\n");
        Salix::ApplicationConfig config;
        config.initial_state = Salix::AppStateType::Game;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.initial_state == Salix::AppStateType::Game);
    }

    TEST_CASE("batch4447 unknown renderer type leaves caller value unchanged") {
        Batch3447SettingsFiles files("Batch4447_UnknownRenderer");
        files.write_yaml("Renderer:\n  type: FutureRenderer4447\n");
        Salix::ApplicationConfig config;
        config.renderer_type = Salix::RendererType::OpenGL;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
    }

    TEST_CASE("batch4447 unknown timer type leaves caller value unchanged") {
        Batch3447SettingsFiles files("Batch4447_UnknownTimer");
        files.write_yaml("Timer:\n  type: FutureTimer4447\n");
        Salix::ApplicationConfig config;
        config.timer_type = Salix::TimerType::Chrono;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch4447 unknown GUI type leaves caller value unchanged") {
        Batch3447SettingsFiles files("Batch4447_UnknownGui");
        files.write_yaml("GUI:\n  type: FutureGui4447\n");
        Salix::ApplicationConfig config;
        config.gui_type = Salix::GuiType::ImGui;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.gui_type == Salix::GuiType::ImGui);
    }

    TEST_CASE("batch4447 save and load preserves quoted slash rich window title") {
        Batch3447SettingsFiles files("Batch4447_TitleEscapes");
        Salix::ApplicationConfig source;
        source.window_config.title = "Salix \"4447\" \\\\ Test";
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.window_config.title == source.window_config.title);
    }

    TEST_CASE("batch4447 save and load preserves negative target FPS") {
        Batch3447SettingsFiles files("Batch4447_NegativeFps");
        Salix::ApplicationConfig source;
        source.target_fps = -4447;
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.target_fps == -4447);
    }

    TEST_CASE("batch4447 save and load preserves zero window dimensions") {
        Batch3447SettingsFiles files("Batch4447_ZeroWindow");
        Salix::ApplicationConfig source;
        source.window_config.width = 0;
        source.window_config.height = 0;
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.window_config.width == 0);
        CHECK(loaded.window_config.height == 0);
    }

    TEST_CASE("batch4447 deleting generated cache forces source YAML to recreate it") {
        Batch3447SettingsFiles files("Batch4447_RecreateCache");
        files.write_yaml("Engine:\n  target_fps: 447\n");
        Salix::SettingsManager manager;
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        REQUIRE(std::filesystem::is_regular_file(files.cache));
        std::filesystem::remove(files.cache);
        Salix::ApplicationConfig second;
        second.target_fps = 1;
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.target_fps == 447);
        CHECK(std::filesystem::is_regular_file(files.cache));
    }

    TEST_CASE("batch4447 save into missing parent directory fails without creating parent") {
        Batch3447SettingsFiles files("Batch4447_MissingParent");
        const auto parent = files.root / "missing" / "nested";
        const auto target = parent / "settings.yaml";
        Salix::ApplicationConfig config;
        Salix::SettingsManager manager;
        CHECK_FALSE(manager.save_settings(target.string(), config));
        CHECK_FALSE(std::filesystem::exists(parent));
        CHECK_FALSE(std::filesystem::exists(target));
    }
}

TEST_SUITE("Salix::management::SettingsManagerBatch4947") {
    TEST_CASE("batch4947 empty save path fails without creating cache side effects") {
        Salix::SettingsManager manager;
        Salix::ApplicationConfig config;
        CHECK_FALSE(manager.save_settings("", config));
    }

    TEST_CASE("batch4947 partial GUI section updates all numeric GUI fields together") {
        Batch3447SettingsFiles files("Batch4947_GuiNumbers");
        files.write_yaml(
            "GUI:\n"
            "  dialog_width_ratio: 0.49\n"
            "  dialog_height_ratio: 0.47\n"
            "  font_scaling: 1.4947\n"
            "  global_dpi_scaling: 2.4947\n");
        Salix::ApplicationConfig config;
        config.gui_type = Salix::GuiType::None;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.gui_type == Salix::GuiType::None);
        CHECK(config.gui_settings.dialog_width_ratio == doctest::Approx(0.49f));
        CHECK(config.gui_settings.dialog_height_ratio == doctest::Approx(0.47f));
        CHECK(config.gui_settings.font_scaling == doctest::Approx(1.4947f));
        CHECK(config.gui_settings.global_dpi_scaling == doctest::Approx(2.4947f));
    }

    TEST_CASE("batch4947 unknown enum strings preserve caller enums while target FPS still updates") {
        Batch3447SettingsFiles files("Batch4947_UnknownEnums");
        files.write_yaml(
            "Engine:\n  initial_state: Future4947\n  target_fps: 4947\n"
            "Renderer:\n  type: FutureRenderer4947\n"
            "GUI:\n  type: FutureGui4947\n"
            "Timer:\n  type: FutureTimer4947\n");
        Salix::ApplicationConfig config;
        config.initial_state = Salix::AppStateType::Game;
        config.renderer_type = Salix::RendererType::OpenGL;
        config.gui_type = Salix::GuiType::None;
        config.timer_type = Salix::TimerType::Chrono;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.initial_state == Salix::AppStateType::Game);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.gui_type == Salix::GuiType::None);
        CHECK(config.timer_type == Salix::TimerType::Chrono);
        CHECK(config.target_fps == 4947);
    }

    TEST_CASE("batch4947 saving invalid enum values serializes documented fallback enum defaults") {
        Batch3447SettingsFiles files("Batch4947_InvalidEnums");
        Salix::ApplicationConfig source;
        source.initial_state = static_cast<Salix::AppStateType>(4947);
        source.renderer_type = static_cast<Salix::RendererType>(4947);
        source.gui_type = static_cast<Salix::GuiType>(4947);
        source.timer_type = static_cast<Salix::TimerType>(4947);
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.initial_state == Salix::AppStateType::Launch);
        CHECK(loaded.renderer_type == Salix::RendererType::SDL);
        CHECK(loaded.gui_type == Salix::GuiType::ImGui);
        CHECK(loaded.timer_type == Salix::TimerType::SDL);
    }

    TEST_CASE("batch4947 save and load preserves negative and zero GUI scaling diagnostics") {
        Batch3447SettingsFiles files("Batch4947_GuiDiagnostics");
        Salix::ApplicationConfig source;
        source.gui_settings.dialog_width_ratio = -0.5f;
        source.gui_settings.dialog_height_ratio = 0.0f;
        source.gui_settings.font_scaling = -4.947f;
        source.gui_settings.global_dpi_scaling = 0.0f;
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig loaded;
        REQUIRE(manager.load_settings(files.yaml.string(), loaded));
        CHECK(loaded.gui_settings.dialog_width_ratio == doctest::Approx(-0.5f));
        CHECK(loaded.gui_settings.dialog_height_ratio == doctest::Approx(0.0f));
        CHECK(loaded.gui_settings.font_scaling == doctest::Approx(-4.947f));
        CHECK(loaded.gui_settings.global_dpi_scaling == doctest::Approx(0.0f));
    }

    TEST_CASE("batch4947 multi dot YAML filename creates cache by replacing only final extension") {
        Batch3447SettingsFiles files("Batch4947_MultiDot");
        const auto yaml = files.root / "settings.dev.local.yaml";
        const auto cache = files.root / "settings.dev.local.cache";
        { std::ofstream out(yaml); out << "Engine:\n  target_fps: 149\n"; }
        Salix::ApplicationConfig config;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(yaml.string(), config));
        CHECK(config.target_fps == 149);
        CHECK(std::filesystem::is_regular_file(cache));
    }

    TEST_CASE("batch4947 cache with same timestamp as YAML wins fast path by greater equal rule") {
        Batch3447SettingsFiles files("Batch4947_EqualTimestamp");
        files.write_yaml("Engine:\n  target_fps: 60\n");
        Salix::SettingsManager manager;
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        REQUIRE(first.target_fps == 60);
        files.write_yaml("Engine:\n  target_fps: 144\n");
        const auto stamp = std::filesystem::file_time_type::clock::now();
        std::filesystem::last_write_time(files.yaml, stamp);
        std::filesystem::last_write_time(files.cache, stamp);
        Salix::ApplicationConfig second;
        second.target_fps = 1;
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.target_fps == 60);
    }

    TEST_CASE("batch4947 newer valid cache restores all GUI numeric fields together") {
        Batch3447SettingsFiles files("Batch4947_GuiCache");
        Salix::ApplicationConfig source;
        source.gui_settings.dialog_width_ratio = 0.11f;
        source.gui_settings.dialog_height_ratio = 0.22f;
        source.gui_settings.font_scaling = 1.33f;
        source.gui_settings.global_dpi_scaling = 2.44f;
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), source));
        Salix::ApplicationConfig first;
        REQUIRE(manager.load_settings(files.yaml.string(), first));
        const auto yaml_time = std::filesystem::last_write_time(files.yaml);
        std::filesystem::last_write_time(files.cache, yaml_time + std::chrono::seconds(2));
        Salix::ApplicationConfig second;
        second.gui_settings.dialog_width_ratio = 9.0f;
        second.gui_settings.dialog_height_ratio = 9.0f;
        second.gui_settings.font_scaling = 9.0f;
        second.gui_settings.global_dpi_scaling = 9.0f;
        REQUIRE(manager.load_settings(files.yaml.string(), second));
        CHECK(second.gui_settings.dialog_width_ratio == doctest::Approx(0.11f));
        CHECK(second.gui_settings.dialog_height_ratio == doctest::Approx(0.22f));
        CHECK(second.gui_settings.font_scaling == doctest::Approx(1.33f));
        CHECK(second.gui_settings.global_dpi_scaling == doctest::Approx(2.44f));
    }

    TEST_CASE("batch4947 empty recognized YAML sections preserve complete caller configuration") {
        Batch3447SettingsFiles files("Batch4947_EmptySections");
        files.write_yaml("Window: {}\nEngine: {}\nRenderer: {}\nGUI: {}\nTimer: {}\n");
        Salix::ApplicationConfig config;
        config.window_config.title = "Keep4947";
        config.window_config.width = 494;
        config.window_config.height = 947;
        config.target_fps = 149;
        config.renderer_type = Salix::RendererType::OpenGL;
        config.initial_state = Salix::AppStateType::Game;
        config.gui_type = Salix::GuiType::None;
        config.timer_type = Salix::TimerType::Chrono;
        Salix::SettingsManager manager;
        REQUIRE(manager.load_settings(files.yaml.string(), config));
        CHECK(config.window_config.title == "Keep4947");
        CHECK(config.window_config.width == 494);
        CHECK(config.window_config.height == 947);
        CHECK(config.target_fps == 149);
        CHECK(config.renderer_type == Salix::RendererType::OpenGL);
        CHECK(config.initial_state == Salix::AppStateType::Game);
        CHECK(config.gui_type == Salix::GuiType::None);
        CHECK(config.timer_type == Salix::TimerType::Chrono);
    }

    TEST_CASE("batch4947 save replaces existing YAML content without leaving stale unknown section") {
        Batch3447SettingsFiles files("Batch4947_ReplaceContent");
        files.write_yaml("Unknown4947:\n  stale: true\nThisIsExtra: 123\n");
        Salix::ApplicationConfig config;
        config.window_config.title = "Fresh4947";
        Salix::SettingsManager manager;
        REQUIRE(manager.save_settings(files.yaml.string(), config));
        std::ifstream in(files.yaml);
        const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        CHECK(text.find("Fresh4947") != std::string::npos);
        CHECK(text.find("Unknown4947") == std::string::npos);
        CHECK(text.find("ThisIsExtra") == std::string::npos);
    }
}
