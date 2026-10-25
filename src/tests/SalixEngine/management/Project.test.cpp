#include <doctest.h>
#include <Salix/management/Project.h>
#include <Salix/rendering/IRenderer.h>
#include <map>
#include <string>
#include <Salix/management/RealmManager.h>
#include <Salix/ecs/Realm.h>
#include <Salix/core/InitContext.h>
#include <filesystem>
#include <fstream>

TEST_SUITE("Salix::management::ProjectBatch1147") {
    TEST_CASE("default project derives stable development defaults") {
        Salix::Project project;
        CHECK(project.get_name() == "TestProject");
        CHECK(project.get_path() == "Sandbox/TestProject");
        CHECK(project.get_project_file_name() == "TestProject.salixproj");
        CHECK(project.get_engine_version() == "0.1.0-Alpha");
        CHECK(project.get_game_dll_name() == "TestProject.dll");
        CHECK(project.get_starting_realm().empty());
        CHECK(project.get_realm_paths().empty());
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("named project derives project and DLL filenames without rewriting root path") {
        Salix::Project project("My Game", "D:/Projects/My Game");
        CHECK(project.get_name() == "My Game");
        CHECK(project.get_path() == "D:/Projects/My Game");
        CHECK(project.get_project_file_name() == "My Game.salixproj");
        CHECK(project.get_game_dll_name() == "My Game.dll");
        CHECK(project.get_engine_version() == "0.1.0-Alpha");
    }

    TEST_CASE("starting realm assignment replaces only starting realm metadata") {
        Salix::Project project("Game", "Root");
        project.set_starting_realm("Menu");
        CHECK(project.get_starting_realm() == "Menu");
        CHECK(project.get_name() == "Game");
        CHECK(project.get_path() == "Root");
        project.set_starting_realm("World");
        CHECK(project.get_starting_realm() == "World");
    }

    TEST_CASE("realm paths are indexed by realm name and preserve independent values") {
        Salix::Project project;
        project.add_realm_path("Menu", "Assets/Realms/Menu.realm");
        project.add_realm_path("World", "Assets/Realms/World.realm");
        const auto& paths = project.get_realm_paths();
        REQUIRE(paths.size() == 2);
        CHECK(paths.at("Menu") == "Assets/Realms/Menu.realm");
        CHECK(paths.at("World") == "Assets/Realms/World.realm");
    }

    TEST_CASE("adding a realm with an existing name replaces its path rather than duplicating the realm") {
        Salix::Project project;
        project.add_realm_path("World", "old.realm");
        project.add_realm_path("World", "new.realm");
        const auto& paths = project.get_realm_paths();
        REQUIRE(paths.size() == 1);
        CHECK(paths.at("World") == "new.realm");
    }

    TEST_CASE("remove realm path removes the matching map value even when file is absent on disk") {
        Salix::Project project;
        const std::string missing = "Batch1147/definitely_missing/World.realm";
        project.add_realm_path("World", missing);
        CHECK(project.remove_realm_path(missing));
        CHECK(project.get_realm_paths().empty());
    }

    TEST_CASE("remove realm path returns false without disturbing unrelated mappings") {
        Salix::Project project;
        project.add_realm_path("Menu", "menu.realm");
        project.add_realm_path("World", "world.realm");
        CHECK_FALSE(project.remove_realm_path("missing.realm"));
        REQUIRE(project.get_realm_paths().size() == 2);
        CHECK(project.get_realm_paths().at("Menu") == "menu.realm");
        CHECK(project.get_realm_paths().at("World") == "world.realm");
    }

    TEST_CASE("load starting realm fails cleanly when no starting realm was configured") {
        Salix::Project project;
        CHECK(project.get_realm_manager() == nullptr);
        CHECK_FALSE(project.load_starting_realm());
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("lifecycle update render and shutdown are no-ops before project initialization") {
        Salix::Project project;
        CHECK_NOTHROW(project.update(0.016f));
        CHECK_NOTHROW(project.update(-1.0f));
        CHECK_NOTHROW(project.render(nullptr));
        CHECK_NOTHROW(project.shutdown());
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("default realm creation is safely ignored until a realm manager exists") {
        Salix::Project project;
        project.add_realm_path("Default", "Default.realm");
        project.set_starting_realm("Default");
        CHECK_NOTHROW(project.create_and_save_default_realm());
        CHECK(project.get_realm_manager() == nullptr);
        REQUIRE(project.get_realm_paths().size() == 1);
        CHECK(project.get_starting_realm() == "Default");
    }
}

TEST_SUITE("Salix::management::ProjectBatch1947") {
    TEST_CASE("empty project name still derives filename suffixes deterministically") {
        Salix::Project project("", "Root");
        CHECK(project.get_name().empty());
        CHECK(project.get_path() == "Root");
        CHECK(project.get_project_file_name() == ".salixproj");
        CHECK(project.get_game_dll_name() == ".dll");
    }

    TEST_CASE("realm map accepts empty key and path as explicit metadata") {
        Salix::Project project;
        project.add_realm_path("", "");
        REQUIRE(project.get_realm_paths().size() == 1);
        CHECK(project.get_realm_paths().at("").empty());
    }

    TEST_CASE("removing one duplicated path removes only the first matching realm mapping") {
        Salix::Project project;
        const std::string path = "__batch1947_shared_missing.realm";
        project.add_realm_path("A", path);
        project.add_realm_path("B", path);
        REQUIRE(project.remove_realm_path(path));
        REQUIRE(project.get_realm_paths().size() == 1);
        CHECK(project.get_realm_paths().begin()->second == path);
    }

    TEST_CASE("removing a mapped path succeeds when the physical file exists") {
        const auto path = std::filesystem::temp_directory_path() / "SalixGameStudio_Batch1947_existing.realm";
        { std::ofstream out(path); out << "{}"; }
        Salix::Project project;
        project.add_realm_path("Existing", path.string());
        CHECK(project.remove_realm_path(path.string()));
        CHECK(project.get_realm_paths().empty());
        std::error_code ec; std::filesystem::remove(path, ec);
    }

    TEST_CASE("initialize constructs a realm manager even when optional services and game DLL are absent") {
        Salix::Project project("Batch1947MissingDllA", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        CHECK(project.get_realm_manager() != nullptr);
        project.shutdown();
    }

    TEST_CASE("initialize materializes configured realm shells before missing DLL return") {
        Salix::Project project("Batch1947MissingDllB", "Root");
        project.add_realm_path("Menu", "Assets/Realms/Menu.realm");
        project.set_starting_realm("Menu");
        Salix::InitContext context{};
        project.initialize(context);
        REQUIRE(project.get_realm_manager() != nullptr);
        CHECK(project.load_starting_realm());
        project.shutdown();
    }

    TEST_CASE("shutdown after initialization releases owned realm manager") {
        Salix::Project project("Batch1947MissingDllC", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        REQUIRE(project.get_realm_manager() != nullptr);
        project.shutdown();
        CHECK(project.get_realm_manager() == nullptr);
    }

    TEST_CASE("update and render remain safe after initialization with no active realm") {
        Salix::Project project("Batch1947MissingDllD", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        CHECK_NOTHROW(project.update(0.5f));
        CHECK_NOTHROW(project.render(nullptr));
        project.shutdown();
    }

    TEST_CASE("project can be initialized again after a completed shutdown") {
        Salix::Project project("Batch1947MissingDllE", "Root");
        Salix::InitContext context{};
        project.initialize(context);
        project.shutdown();
        REQUIRE(project.get_realm_manager() == nullptr);
        project.initialize(context);
        CHECK(project.get_realm_manager() != nullptr);
        project.shutdown();
    }

    TEST_CASE("latest starting realm selection is used after initialization") {
        Salix::Project project("Batch1947MissingDllF", "Root");
        project.add_realm_path("First", "First.realm");
        project.add_realm_path("Second", "Second.realm");
        project.set_starting_realm("First");
        project.set_starting_realm("Second");
        Salix::InitContext context{};
        project.initialize(context);
        CHECK(project.get_starting_realm() == "Second");
        CHECK(project.load_starting_realm());
        project.shutdown();
    }
}

TEST_SUITE("Salix::management::ProjectBatch2347") {
    TEST_CASE("batch2347 default project exposes canonical derived file and DLL names") {
        Salix::Project project;
        CHECK(project.get_name()=="TestProject");
        CHECK(project.get_path()=="Sandbox/TestProject");
        CHECK(project.get_project_file_name()=="TestProject.salixproj");
        CHECK(project.get_game_dll_name()=="TestProject.dll");
        CHECK(project.get_engine_version()=="0.1.0-Alpha");
    }
    TEST_CASE("batch2347 custom constructor derives file and DLL names from exact project name") {
        Salix::Project project("Project 2347","D:/Projects/Project 2347");
        CHECK(project.get_name()=="Project 2347");
        CHECK(project.get_path()=="D:/Projects/Project 2347");
        CHECK(project.get_project_file_name()=="Project 2347.salixproj");
        CHECK(project.get_game_dll_name()=="Project 2347.dll");
    }
    TEST_CASE("batch2347 realm path insertion stores independent name-to-path entries") {
        Salix::Project project; project.add_realm_path("Menu","Scenes/Menu.realm"); project.add_realm_path("Game","Scenes/Game.realm");
        const auto& paths=project.get_realm_paths(); REQUIRE(paths.size()==2); CHECK(paths.at("Menu")=="Scenes/Menu.realm"); CHECK(paths.at("Game")=="Scenes/Game.realm");
    }
    TEST_CASE("batch2347 duplicate realm name replaces only its mapped path") {
        Salix::Project project; project.add_realm_path("World","Old.realm"); project.add_realm_path("Other","Other.realm"); project.add_realm_path("World","New.realm");
        const auto& paths=project.get_realm_paths(); REQUIRE(paths.size()==2); CHECK(paths.at("World")=="New.realm"); CHECK(paths.at("Other")=="Other.realm");
    }
    TEST_CASE("batch2347 removing mapped path succeeds even when backing file is absent") {
        Salix::Project project; const std::string path="Batch2347_missing_realm_path.realm"; project.add_realm_path("Missing",path);
        REQUIRE(project.get_realm_paths().count("Missing")==1); CHECK(project.remove_realm_path(path)); CHECK(project.get_realm_paths().empty());
    }
    TEST_CASE("batch2347 removing unknown path returns false without disturbing map") {
        Salix::Project project; project.add_realm_path("Keep","Keep.realm");
        CHECK_FALSE(project.remove_realm_path("Unknown.realm")); REQUIRE(project.get_realm_paths().size()==1); CHECK(project.get_realm_paths().at("Keep")=="Keep.realm");
    }
    TEST_CASE("batch2347 starting realm selection replaces complete previous value") {
        Salix::Project project; project.set_starting_realm("Menu"); CHECK(project.get_starting_realm()=="Menu"); project.set_starting_realm("Gameplay"); CHECK(project.get_starting_realm()=="Gameplay"); project.set_starting_realm(""); CHECK(project.get_starting_realm().empty());
    }
    TEST_CASE("batch2347 load starting realm rejects empty selection before initialization") {
        Salix::Project project; CHECK(project.get_starting_realm().empty()); CHECK_FALSE(project.load_starting_realm());
    }
    TEST_CASE("batch2347 initialization materializes configured realm shells before DLL load result") {
        Salix::Project project("Batch2347MissingDll","Root2347"); project.add_realm_path("Menu","Menu.realm"); project.add_realm_path("Game","Game.realm"); Salix::InitContext context{};
        project.initialize(context); auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr); CHECK(manager->set_active_realm("Menu")); CHECK(manager->get_active_realm()!=nullptr); CHECK(manager->get_active_realm()->get_path()=="Menu.realm"); project.shutdown();
    }
    TEST_CASE("batch2347 shutdown is idempotent and update render stay safe afterward") {
        Salix::Project project("Batch2347MissingDll2","Root2347"); Salix::InitContext context{}; project.initialize(context); REQUIRE(project.get_realm_manager()!=nullptr);
        project.shutdown(); CHECK(project.get_realm_manager()==nullptr); CHECK_NOTHROW(project.shutdown()); CHECK_NOTHROW(project.update(-1.f)); CHECK_NOTHROW(project.render(nullptr));
    }
}

TEST_SUITE("Salix::management::ProjectBatch2547") {
    TEST_CASE("batch2547 empty project name derives empty-prefixed filenames without rewriting root") {
        Salix::Project project("","Root2547");
        CHECK(project.get_name().empty()); CHECK(project.get_path()=="Root2547");
        CHECK(project.get_project_file_name()==".salixproj"); CHECK(project.get_game_dll_name()==".dll");
    }
    TEST_CASE("batch2547 project name punctuation is preserved verbatim in derived filenames") {
        Salix::Project project("Game.Name-2547","D:/Root");
        CHECK(project.get_project_file_name()=="Game.Name-2547.salixproj");
        CHECK(project.get_game_dll_name()=="Game.Name-2547.dll");
    }
    TEST_CASE("batch2547 repeated starting realm assignment keeps only latest exact string") {
        Salix::Project project; project.set_starting_realm("One"); project.set_starting_realm("Two"); project.set_starting_realm("Realm 2547");
        CHECK(project.get_starting_realm()=="Realm 2547");
    }
    TEST_CASE("batch2547 realm path map supports empty path values without rejecting metadata") {
        Salix::Project project; project.add_realm_path("EmptyPath","");
        REQUIRE(project.get_realm_paths().count("EmptyPath")==1); CHECK(project.get_realm_paths().at("EmptyPath").empty());
    }
    TEST_CASE("batch2547 realm path map supports empty realm name as exact key") {
        Salix::Project project; project.add_realm_path("","NoName.realm");
        REQUIRE(project.get_realm_paths().count("")==1); CHECK(project.get_realm_paths().at("")=="NoName.realm");
    }
    TEST_CASE("batch2547 removing one of duplicate path values removes exactly one map entry") {
        Salix::Project project; const std::string shared="Batch2547Shared.realm";
        project.add_realm_path("A",shared); project.add_realm_path("B",shared); REQUIRE(project.get_realm_paths().size()==2);
        CHECK(project.remove_realm_path(shared)); CHECK(project.get_realm_paths().size()==1);
        CHECK(project.get_realm_paths().begin()->second==shared);
    }
    TEST_CASE("batch2547 removing same mapped path twice reports success then failure") {
        Salix::Project project; const std::string path="Batch2547Twice.realm"; project.add_realm_path("Only",path);
        CHECK(project.remove_realm_path(path)); CHECK_FALSE(project.remove_realm_path(path)); CHECK(project.get_realm_paths().empty());
    }
    TEST_CASE("batch2547 remove path uses exact case-sensitive string matching") {
        Salix::Project project; project.add_realm_path("Case","Assets/Realm2547.realm");
        CHECK_FALSE(project.remove_realm_path("assets/realm2547.realm")); REQUIRE(project.get_realm_paths().size()==1);
        CHECK(project.get_realm_paths().at("Case")=="Assets/Realm2547.realm");
    }
    TEST_CASE("batch2547 preinitialization lifecycle calls preserve all metadata collections") {
        Salix::Project project("Meta2547","Root2547"); project.set_starting_realm("Start"); project.add_realm_path("Start","Start.realm");
        project.update(123.f); project.render(nullptr); project.shutdown();
        CHECK(project.get_name()=="Meta2547"); CHECK(project.get_path()=="Root2547"); CHECK(project.get_starting_realm()=="Start");
        REQUIRE(project.get_realm_paths().size()==1); CHECK(project.get_realm_manager()==nullptr);
    }
    TEST_CASE("batch2547 create default realm before initialization does not mutate configured realm metadata") {
        Salix::Project project; project.add_realm_path("Default","ConfiguredDefault.realm"); project.set_starting_realm("Default");
        project.create_and_save_default_realm();
        CHECK(project.get_starting_realm()=="Default"); REQUIRE(project.get_realm_paths().size()==1);
        CHECK(project.get_realm_paths().at("Default")=="ConfiguredDefault.realm"); CHECK(project.get_realm_manager()==nullptr);
    }
}

TEST_SUITE("Salix::management::ProjectBatch2647") {
    TEST_CASE("batch2647 custom project preserves root with spaces and punctuation verbatim") {
        Salix::Project project("Project-2647","D:/My Projects/Project-2647");
        CHECK(project.get_name()=="Project-2647"); CHECK(project.get_path()=="D:/My Projects/Project-2647");
        CHECK(project.get_project_file_name()=="Project-2647.salixproj"); CHECK(project.get_game_dll_name()=="Project-2647.dll");
    }
    TEST_CASE("batch2647 realm map orders independent keys while preserving exact values") {
        Salix::Project project; project.add_realm_path("Zulu","Z.realm"); project.add_realm_path("Alpha","A.realm"); project.add_realm_path("Middle","M.realm");
        const auto& paths=project.get_realm_paths(); REQUIRE(paths.size()==3);
        auto it=paths.begin(); CHECK(it->first=="Alpha"); ++it; CHECK(it->first=="Middle"); ++it; CHECK(it->first=="Zulu");
    }
    TEST_CASE("batch2647 duplicate shared paths remove first map entry in key iteration order") {
        Salix::Project project; const std::string shared="Batch2647Shared.realm";
        project.add_realm_path("B",shared); project.add_realm_path("A",shared); REQUIRE(project.get_realm_paths().size()==2);
        REQUIRE(project.remove_realm_path(shared)); REQUIRE(project.get_realm_paths().size()==1);
        CHECK(project.get_realm_paths().count("A")==0); CHECK(project.get_realm_paths().count("B")==1);
    }
    TEST_CASE("batch2647 removing path that exists on disk also removes matching project entry") {
        const auto path=std::filesystem::temp_directory_path()/"SalixGameStudio_Batch2647_Existing.realm";
        { std::ofstream out(path); out<<"realm"; }
        Salix::Project project; project.add_realm_path("Existing",path.string());
        REQUIRE(project.remove_realm_path(path.string())); CHECK(project.get_realm_paths().empty());
        std::error_code ec; std::filesystem::remove(path,ec);
    }
    TEST_CASE("batch2647 initialization propagates exact project root into RealmManager") {
        Salix::Project project("Batch2647MissingDllRoot","Root/Batch2647"); Salix::InitContext context{};
        project.initialize(context); auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr);
        CHECK(manager->get_project_root_path()=="Root/Batch2647"); project.shutdown();
    }
    TEST_CASE("batch2647 initialization materializes every configured realm shell") {
        Salix::Project project("Batch2647MissingDllShells","Root2647");
        project.add_realm_path("Menu","Scenes/Menu2647.realm"); project.add_realm_path("Game","Scenes/Game2647.realm");
        Salix::InitContext context{}; project.initialize(context); auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr);
        REQUIRE(manager->set_active_realm("Menu")); CHECK(manager->get_active_realm()->get_path()=="Scenes/Menu2647.realm");
        REQUIRE(manager->set_active_realm("Game")); CHECK(manager->get_active_realm()->get_path()=="Scenes/Game2647.realm");
        project.shutdown();
    }
    TEST_CASE("batch2647 shutdown preserves configured metadata and realm path map") {
        Salix::Project project("Batch2647MissingDllShutdown","Root2647"); project.set_starting_realm("Main"); project.add_realm_path("Main","Main2647.realm");
        Salix::InitContext context{}; project.initialize(context); project.shutdown();
        CHECK(project.get_realm_manager()==nullptr); CHECK(project.get_name()=="Batch2647MissingDllShutdown"); CHECK(project.get_path()=="Root2647");
        CHECK(project.get_starting_realm()=="Main"); REQUIRE(project.get_realm_paths().size()==1); CHECK(project.get_realm_paths().at("Main")=="Main2647.realm");
    }
    TEST_CASE("batch2647 reinitialization rebuilds realm shells from persistent metadata") {
        Salix::Project project("Batch2647MissingDllReinit","Root2647"); project.add_realm_path("Persistent","Persistent2647.realm");
        Salix::InitContext context{}; project.initialize(context); project.shutdown(); REQUIRE(project.get_realm_manager()==nullptr);
        project.initialize(context); auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr); REQUIRE(manager->set_active_realm("Persistent"));
        CHECK(manager->get_active_realm()->get_path()=="Persistent2647.realm"); project.shutdown();
    }
    TEST_CASE("batch2647 latest starting realm can be loaded from rebuilt configured shells") {
        Salix::Project project("Batch2647MissingDllStart","Root2647"); project.add_realm_path("First","First2647.realm"); project.add_realm_path("Second","Second2647.realm");
        project.set_starting_realm("First"); project.set_starting_realm("Second"); Salix::InitContext context{}; project.initialize(context);
        CHECK(project.load_starting_realm()); REQUIRE(project.get_realm_manager()!=nullptr); REQUIRE(project.get_realm_manager()->get_active_realm()!=nullptr);
        CHECK(project.get_realm_manager()->get_active_realm()->get_name()=="Second"); project.shutdown();
    }
    TEST_CASE("batch2647 default realm creation before initialization is a metadata preserving no-op") {
        Salix::Project project("NoInit2647","Root2647"); project.add_realm_path("Default","Configured2647.realm"); project.set_starting_realm("Default");
        project.create_and_save_default_realm();
        CHECK(project.get_realm_manager()==nullptr); CHECK(project.get_starting_realm()=="Default"); REQUIRE(project.get_realm_paths().size()==1); CHECK(project.get_realm_paths().at("Default")=="Configured2647.realm");
    }
}

TEST_SUITE("Salix::management::ProjectBatch2747") {
    TEST_CASE("batch2747 default project exposes canonical derived file and DLL names") {
        Salix::Project project; CHECK(project.get_name()=="TestProject"); CHECK(project.get_path()=="Sandbox/TestProject");
        CHECK(project.get_project_file_name()=="TestProject.salixproj"); CHECK(project.get_game_dll_name()=="TestProject.dll");
    }
    TEST_CASE("batch2747 custom project derives file and DLL names directly from project name") {
        Salix::Project project("Game 2747","D:/Projects/Game 2747"); CHECK(project.get_project_file_name()=="Game 2747.salixproj"); CHECK(project.get_game_dll_name()=="Game 2747.dll");
    }
    TEST_CASE("batch2747 starting realm accepts explicit empty assignment") {
        Salix::Project project; project.set_starting_realm("Realm"); project.set_starting_realm(""); CHECK(project.get_starting_realm().empty());
    }
    TEST_CASE("batch2747 adding same realm name replaces previous path") {
        Salix::Project project; project.add_realm_path("Main","Old.realm"); project.add_realm_path("Main","New.realm");
        REQUIRE(project.get_realm_paths().size()==1); CHECK(project.get_realm_paths().at("Main")=="New.realm");
    }
    TEST_CASE("batch2747 remove realm path succeeds from map even when disk file is absent") {
        Salix::Project project; const std::string path="Batch2747_missing_but_mapped.realm"; project.add_realm_path("Mapped",path);
        CHECK(project.remove_realm_path(path)); CHECK(project.get_realm_paths().empty());
    }
    TEST_CASE("batch2747 remove unknown realm path returns false without mutating other entries") {
        Salix::Project project; project.add_realm_path("Keep","Keep2747.realm");
        CHECK_FALSE(project.remove_realm_path("Missing2747.realm")); REQUIRE(project.get_realm_paths().size()==1); CHECK(project.get_realm_paths().at("Keep")=="Keep2747.realm");
    }
    TEST_CASE("batch2747 initialization with empty context still creates realm manager and root metadata") {
        Salix::Project project("Batch2747MissingDllInit","Root2747"); Salix::InitContext context{}; project.initialize(context);
        auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr); CHECK(manager->get_project_root_path()=="Root2747"); project.shutdown();
    }
    TEST_CASE("batch2747 initialized project with no starting realm rejects starting realm load") {
        Salix::Project project("Batch2747MissingDllEmptyStart","Root2747"); Salix::InitContext context{}; project.initialize(context);
        CHECK_FALSE(project.load_starting_realm()); project.shutdown();
    }
    TEST_CASE("batch2747 initialized configured starting realm selects its shell successfully") {
        Salix::Project project("Batch2747MissingDllStart","Root2747"); project.add_realm_path("Main2747","Scenes/Main2747.realm"); project.set_starting_realm("Main2747");
        Salix::InitContext context{}; project.initialize(context); REQUIRE(project.load_starting_realm()); REQUIRE(project.get_realm_manager()!=nullptr);
        REQUIRE(project.get_realm_manager()->get_active_realm()!=nullptr); CHECK(project.get_realm_manager()->get_active_realm()->get_name()=="Main2747"); project.shutdown();
    }
    TEST_CASE("batch2747 shutdown remains idempotent after initialization") {
        Salix::Project project("Batch2747MissingDllShutdown","Root2747"); Salix::InitContext context{}; project.initialize(context); REQUIRE(project.get_realm_manager()!=nullptr);
        project.shutdown(); CHECK(project.get_realm_manager()==nullptr); CHECK_NOTHROW(project.shutdown()); CHECK(project.get_realm_manager()==nullptr);
    }
}

TEST_SUITE("Salix::management::ProjectBatch2947") {
    TEST_CASE("batch2947 independent realm names may intentionally share the same path") {
        Salix::Project project; project.add_realm_path("A2947","Shared2947.realm"); project.add_realm_path("B2947","Shared2947.realm");
        REQUIRE(project.get_realm_paths().size()==2); CHECK(project.get_realm_paths().at("A2947")=="Shared2947.realm"); CHECK(project.get_realm_paths().at("B2947")=="Shared2947.realm");
    }
    TEST_CASE("batch2947 removing shared realm path removes one matching mapping per call") {
        Salix::Project project; project.add_realm_path("A2947","Shared2947.realm"); project.add_realm_path("B2947","Shared2947.realm");
        CHECK(project.remove_realm_path("Shared2947.realm")); CHECK(project.get_realm_paths().size()==1);
        CHECK(project.remove_realm_path("Shared2947.realm")); CHECK(project.get_realm_paths().empty());
    }
    TEST_CASE("batch2947 removing same unique path twice succeeds then reports absence") {
        Salix::Project project; project.add_realm_path("Only2947","Only2947.realm");
        CHECK(project.remove_realm_path("Only2947.realm")); CHECK_FALSE(project.remove_realm_path("Only2947.realm")); CHECK(project.get_realm_paths().empty());
    }
    TEST_CASE("batch2947 starting realm replacement never mutates realm path map") {
        Salix::Project project; project.add_realm_path("One","One.realm"); project.add_realm_path("Two","Two.realm");
        project.set_starting_realm("One"); project.set_starting_realm("Two");
        CHECK(project.get_starting_realm()=="Two"); CHECK(project.get_realm_paths().size()==2);
    }
    TEST_CASE("batch2947 initialize rebuilds configured realm shells without altering metadata") {
        Salix::Project project("Batch2947MissingDllInit","Root2947"); project.add_realm_path("Menu2947","Menu2947.realm"); project.add_realm_path("Game2947","Game2947.realm");
        Salix::InitContext context{}; project.initialize(context); auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr);
        REQUIRE(manager->set_active_realm("Menu2947")); REQUIRE(manager->get_active_realm()!=nullptr); CHECK(manager->get_active_realm()->get_path()=="Menu2947.realm");
        CHECK(project.get_realm_paths().size()==2); project.shutdown();
    }
    TEST_CASE("batch2947 initialized missing starting realm returns false without clearing manager") {
        Salix::Project project("Batch2947MissingDllUnknownStart","Root2947"); project.add_realm_path("Known2947","Known2947.realm"); project.set_starting_realm("Unknown2947");
        Salix::InitContext context{}; project.initialize(context); auto* manager=project.get_realm_manager(); REQUIRE(manager!=nullptr);
        CHECK_FALSE(project.load_starting_realm()); CHECK(project.get_realm_manager()==manager); project.shutdown();
    }
    TEST_CASE("batch2947 successful starting realm load can be repeated idempotently") {
        Salix::Project project("Batch2947MissingDllRepeatStart","Root2947"); project.add_realm_path("Main2947","Main2947.realm"); project.set_starting_realm("Main2947");
        Salix::InitContext context{}; project.initialize(context); REQUIRE(project.load_starting_realm()); auto* first=project.get_realm_manager()->get_active_realm(); REQUIRE(first!=nullptr);
        REQUIRE(project.load_starting_realm()); CHECK(project.get_realm_manager()->get_active_realm()==first); project.shutdown();
    }
    TEST_CASE("batch2947 shutdown preserves persistent project metadata and realm mappings") {
        Salix::Project project("Batch2947MissingDllShutdown","Root2947"); project.add_realm_path("Main2947","Main2947.realm"); project.set_starting_realm("Main2947"); Salix::InitContext context{};
        project.initialize(context); project.shutdown(); CHECK(project.get_realm_manager()==nullptr); CHECK(project.get_name()=="Batch2947MissingDllShutdown"); CHECK(project.get_path()=="Root2947");
        CHECK(project.get_starting_realm()=="Main2947"); CHECK(project.get_realm_paths().at("Main2947")=="Main2947.realm");
    }
    TEST_CASE("batch2947 update and render become harmless again after shutdown") {
        Salix::Project project("Batch2947MissingDllLifecycle","Root2947"); Salix::InitContext context{}; project.initialize(context); project.shutdown();
        CHECK_NOTHROW(project.update(1.0f)); CHECK_NOTHROW(project.render(nullptr)); CHECK(project.get_realm_manager()==nullptr);
    }
    TEST_CASE("batch2947 derived filenames remain stable across realm metadata mutations") {
        Salix::Project project("Stable2947","D:/Root2947"); const auto project_file=project.get_project_file_name(); const auto dll=project.get_game_dll_name();
        project.add_realm_path("R","R.realm"); project.set_starting_realm("R"); project.remove_realm_path("R.realm");
        CHECK(project.get_project_file_name()==project_file); CHECK(project.get_game_dll_name()==dll); CHECK(project.get_name()=="Stable2947"); CHECK(project.get_path()=="D:/Root2947");
    }
}

