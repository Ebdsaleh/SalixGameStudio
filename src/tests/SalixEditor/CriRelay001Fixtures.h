#pragma once
#include <doctest.h>
#include <Editor/Archetypes.h>
#include <Salix/reflection/ByteMirror.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace EditorCri {
    using namespace Salix;
    inline SimpleGuid id(uint64_t value) { return SimpleGuid::from_value(value); }
    inline void register_types() {
        static const bool registered = [] { ByteMirror::register_all_types(); return true; }();
        (void)registered;
    }
    inline EntityArchetype entity(uint64_t value = 10001, const char* name = "Player") {
        EntityArchetype e; e.id = id(value); e.name = name; return e;
    }
    inline ElementArchetype element(const char* type = "Transform", uint64_t value = 20001,
                                    const char* yaml = "{name: Transform}") {
        ElementArchetype e; e.id = id(value); e.owner_id = id(10001);
        e.type_name = type; e.name = type; e.data = YAML::Load(yaml); return e;
    }
    // Each case gets an exclusively created directory; cleanup is limited to that directory.
    class TempRealm {
        std::filesystem::path dir;
    public:
        TempRealm() {
            static std::atomic<unsigned long> serial{0};
            const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
            for (int n = 0; n < 100; ++n) {
                auto candidate = std::filesystem::temp_directory_path() /
                    ("salix-editor-cri-" + std::to_string(tick) + "-" + std::to_string(serial++));
                if (std::filesystem::create_directory(candidate)) { dir = candidate; return; }
            }
            throw std::runtime_error("Cannot create unique Editor CRI fixture directory");
        }
        TempRealm(const TempRealm&) = delete;
        TempRealm& operator=(const TempRealm&) = delete;
        ~TempRealm() { std::error_code ec; std::filesystem::remove_all(dir, ec); }
        std::string path() const { return (dir / "fixture.realm").string(); }
        std::string write(const std::string& yaml) const {
            std::ofstream out(path(), std::ios::binary);
            out << yaml;
            out.close();
            if (!out) throw std::runtime_error("Cannot write Editor CRI fixture");
            return path();
        }
    };
}
