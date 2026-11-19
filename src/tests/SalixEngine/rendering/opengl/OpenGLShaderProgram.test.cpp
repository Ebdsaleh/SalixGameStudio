#include <doctest.h>
#include <Salix/rendering/opengl/OpenGLShaderProgram.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <system_error>

TEST_SUITE("Salix::rendering::OpenGLShaderProgramBatch1647") {
    TEST_CASE("shader program is not default constructible without shader source paths") {
        CHECK_FALSE((std::is_default_constructible_v<Salix::OpenGLShaderProgram>));
    }

    TEST_CASE("shader program is constructible from vertex and fragment path strings") {
        CHECK((std::is_constructible_v<
            Salix::OpenGLShaderProgram,
            const std::string&,
            const std::string&>));
    }

    TEST_CASE("shader program remains destructible through its exported class contract") {
        CHECK((std::is_destructible_v<Salix::OpenGLShaderProgram>));
    }

    TEST_CASE("shader program is not copy constructible because implementation ownership is unique") {
        CHECK_FALSE((std::is_copy_constructible_v<Salix::OpenGLShaderProgram>));
    }

    TEST_CASE("shader program is not copy assignable because implementation ownership is unique") {
        CHECK_FALSE((std::is_copy_assignable_v<Salix::OpenGLShaderProgram>));
    }

    TEST_CASE("shader program is not implicitly move constructible with current user declared destructor") {
        CHECK_FALSE((std::is_move_constructible_v<Salix::OpenGLShaderProgram>));
    }

    TEST_CASE("shader program public ID retains GLuint representation") {
        using IdType = decltype(std::declval<Salix::OpenGLShaderProgram&>().ID);
        CHECK((std::is_same_v<IdType, GLuint>));
    }

    TEST_CASE("shader program use method remains a const void operation") {
        using Return = decltype(std::declval<const Salix::OpenGLShaderProgram&>().use());
        CHECK((std::is_same_v<Return, void>));
    }

    TEST_CASE("shader program matrix and integer setters retain void command semantics") {
        using MatReturn = decltype(
            std::declval<const Salix::OpenGLShaderProgram&>().setMat4(
                std::declval<const std::string&>(),
                std::declval<const glm::mat4&>()));
        using IntReturn = decltype(
            std::declval<const Salix::OpenGLShaderProgram&>().setInt(
                std::declval<const std::string&>(),
                1));
        CHECK((std::is_same_v<MatReturn, void>));
        CHECK((std::is_same_v<IntReturn, void>));
    }

    TEST_CASE("shader program vector setters retain vec4 and vec3 const-reference contracts") {
        using Vec4Return = decltype(
            std::declval<const Salix::OpenGLShaderProgram&>().setVec4(
                std::declval<const std::string&>(),
                std::declval<const glm::vec4&>()));
        using Vec3Return = decltype(
            std::declval<const Salix::OpenGLShaderProgram&>().setVec3(
                std::declval<const std::string&>(),
                std::declval<const glm::vec3&>()));
        CHECK((std::is_same_v<Vec4Return, void>));
        CHECK((std::is_same_v<Vec3Return, void>));
    }
}


namespace {
class Batch7366ShaderFiles {
public:
    Batch7366ShaderFiles() {
        static std::atomic<unsigned long> counter{0};
        std::error_code ec;
        const auto base = std::filesystem::temp_directory_path(ec);
        if (ec) {
            return;
        }

        root = base / ("SalixGameStudio_Batch7366_Shader_" + std::to_string(++counter));
        std::filesystem::create_directories(root, ec);
        ready = !ec;
    }

    ~Batch7366ShaderFiles() {
        std::error_code ec;
        if (!root.empty()) {
            std::filesystem::remove_all(root, ec);
        }
    }

    std::filesystem::path file(const std::string& name) const {
        return root / name;
    }

    void write(const std::string& name, const std::string& contents) const {
        std::ofstream stream(file(name), std::ios::out | std::ios::trunc);
        stream << contents;
    }

    bool ready = false;
    std::filesystem::path root;
};

const char* batch7366_shader_source =
    "#version 450 core\n"
    "void main() {}\n";
}

TEST_SUITE("Salix::rendering::OpenGLShaderProgramBatch7366") {
    TEST_CASE("batch7366 two missing shader files produce inert program without GL dispatch") {
        Batch7366ShaderFiles files;
        REQUIRE(files.ready);

        const auto vertex = files.file("missing.vert");
        const auto fragment = files.file("missing.frag");
        REQUIRE_FALSE(std::filesystem::exists(vertex));
        REQUIRE_FALSE(std::filesystem::exists(fragment));

        Salix::OpenGLShaderProgram program(vertex.string(), fragment.string());
        CHECK(program.ID == 0);
    }

    TEST_CASE("batch7366 readable vertex with missing fragment still produces inert program") {
        Batch7366ShaderFiles files;
        REQUIRE(files.ready);
        files.write("valid.vert", batch7366_shader_source);

        const auto vertex = files.file("valid.vert");
        const auto fragment = files.file("missing.frag");
        REQUIRE(std::filesystem::exists(vertex));
        REQUIRE_FALSE(std::filesystem::exists(fragment));

        Salix::OpenGLShaderProgram program(vertex.string(), fragment.string());
        CHECK(program.ID == 0);
    }

    TEST_CASE("batch7366 missing vertex with readable fragment still produces inert program") {
        Batch7366ShaderFiles files;
        REQUIRE(files.ready);
        files.write("valid.frag", batch7366_shader_source);

        const auto vertex = files.file("missing.vert");
        const auto fragment = files.file("valid.frag");
        REQUIRE_FALSE(std::filesystem::exists(vertex));
        REQUIRE(std::filesystem::exists(fragment));

        Salix::OpenGLShaderProgram program(vertex.string(), fragment.string());
        CHECK(program.ID == 0);
    }

    TEST_CASE("batch7366 empty vertex file is rejected before any GL dispatch") {
        Batch7366ShaderFiles files;
        REQUIRE(files.ready);
        files.write("empty.vert", "");
        files.write("valid.frag", batch7366_shader_source);

        Salix::OpenGLShaderProgram program(
            files.file("empty.vert").string(),
            files.file("valid.frag").string());

        CHECK(program.ID == 0);
    }

    TEST_CASE("batch7366 empty fragment file is rejected before any GL dispatch") {
        Batch7366ShaderFiles files;
        REQUIRE(files.ready);
        files.write("valid.vert", batch7366_shader_source);
        files.write("empty.frag", "");

        Salix::OpenGLShaderProgram program(
            files.file("valid.vert").string(),
            files.file("empty.frag").string());

        CHECK(program.ID == 0);
    }

    TEST_CASE("batch7366 two readable but empty shader files remain inert and destruct safely") {
        Batch7366ShaderFiles files;
        REQUIRE(files.ready);
        files.write("empty.vert", "");
        files.write("empty.frag", "");

        {
            Salix::OpenGLShaderProgram program(
                files.file("empty.vert").string(),
                files.file("empty.frag").string());
            CHECK(program.ID == 0);
        }

        CHECK(std::filesystem::exists(files.file("empty.vert")));
        CHECK(std::filesystem::exists(files.file("empty.frag")));
    }
}

TEST_SUITE("Salix::rendering::OpenGLShaderProgramBatch7478") {
    TEST_CASE("batch7478 two nested missing shader paths stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); Salix::OpenGLShaderProgram program(files.file("missing/a.vert").string(),files.file("missing/b.frag").string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 directory vertex and missing fragment stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); Salix::OpenGLShaderProgram program(files.root.string(),files.file("missing.frag").string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 missing vertex and directory fragment stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); Salix::OpenGLShaderProgram program(files.file("missing.vert").string(),files.root.string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 two directory shader paths stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); Salix::OpenGLShaderProgram program(files.root.string(),files.root.string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 empty vertex and missing fragment stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); files.write("empty.vert",""); Salix::OpenGLShaderProgram program(files.file("empty.vert").string(),files.file("missing.frag").string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 missing vertex and empty fragment stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); files.write("empty.frag",""); Salix::OpenGLShaderProgram program(files.file("missing.vert").string(),files.file("empty.frag").string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 empty vertex and directory fragment stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); files.write("empty.vert",""); Salix::OpenGLShaderProgram program(files.file("empty.vert").string(),files.root.string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 directory vertex and empty fragment stay inert") { Batch7366ShaderFiles files; REQUIRE(files.ready); files.write("empty.frag",""); Salix::OpenGLShaderProgram program(files.root.string(),files.file("empty.frag").string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 valid vertex and nested missing fragment stop before GL dispatch") { Batch7366ShaderFiles files; REQUIRE(files.ready); files.write("valid.vert",batch7366_shader_source); Salix::OpenGLShaderProgram program(files.file("valid.vert").string(),files.file("missing/deep.frag").string()); CHECK(program.ID==0); }
    TEST_CASE("batch7478 nested missing vertex and valid fragment stop before GL dispatch") { Batch7366ShaderFiles files; REQUIRE(files.ready); files.write("valid.frag",batch7366_shader_source); Salix::OpenGLShaderProgram program(files.file("missing/deep.vert").string(),files.file("valid.frag").string()); CHECK(program.ID==0); }
}

