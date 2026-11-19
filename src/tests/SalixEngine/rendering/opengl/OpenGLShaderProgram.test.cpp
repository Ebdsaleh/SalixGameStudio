#include <doctest.h>
#include <Salix/rendering/opengl/OpenGLShaderProgram.h>
#include <Salix/rendering/opengl/OpenGLRenderer.h>
#include <SDL.h>
#include <cstdlib>
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

namespace {
struct Batch7778ShaderGLContext {
    std::filesystem::path previous=std::filesystem::current_path();
    bool had=false;std::string old;bool sdl_ready=false;
    Salix::OpenGLRenderer renderer; bool ready=false;
    Batch7778ShaderGLContext(){
        auto p=std::filesystem::absolute(std::filesystem::path(__FILE__)).parent_path();
        while(p.has_parent_path()&&!std::filesystem::exists(p/"Assets"/"Shaders"))p=p.parent_path();
        if(std::filesystem::exists(p/"Assets"/"Shaders"))std::filesystem::current_path(p);
        if(const char* v=SDL_getenv("SDL_VIDEODRIVER")){had=true;old=v;}
        SDL_Quit();SDL_setenv("SDL_VIDEODRIVER","windows",1);sdl_ready=SDL_Init(SDL_INIT_VIDEO)==0;
        if(sdl_ready){Salix::WindowConfig cfg;cfg.title="Batch7778 Shader";cfg.width=96;cfg.height=64;ready=renderer.initialize(cfg);}
    }
    ~Batch7778ShaderGLContext(){renderer.shutdown();SDL_Quit();if(had)SDL_setenv("SDL_VIDEODRIVER",old.c_str(),1);else _putenv_s("SDL_VIDEODRIVER","");std::error_code ec;std::filesystem::current_path(previous,ec);}
};
}

TEST_SUITE("Salix::rendering::OpenGLShaderProgramBatch7778") {
    TEST_CASE("batch7778 textured shader compiles and links in real DLL GL context") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag"); CHECK(p.ID!=0u);
    }
    TEST_CASE("batch7778 color shader compiles and links in real DLL GL context") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/color.vert","Assets/Shaders/OpenGL/2D/color.frag"); CHECK(p.ID!=0u);
    }
    TEST_CASE("batch7778 simple 3D shader compiles and links in real DLL GL context") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/3D/simple.vert","Assets/Shaders/OpenGL/3D/color_only.frag"); CHECK(p.ID!=0u);
    }
    TEST_CASE("batch7778 valid shader use executes GL program binding") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag"); REQUIRE(p.ID!=0u); CHECK_NOTHROW(p.use());
    }
    TEST_CASE("batch7778 valid shader sets sampler integer uniform") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag"); REQUIRE(p.ID!=0u); CHECK_NOTHROW(p.setInt("texture_sampler",0));
    }
    TEST_CASE("batch7778 valid shader sets model matrix uniform") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag"); REQUIRE(p.ID!=0u); CHECK_NOTHROW(p.setMat4("model",glm::mat4(1.0f)));
    }
    TEST_CASE("batch7778 valid shader sets tint vec4 uniform") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag"); REQUIRE(p.ID!=0u); CHECK_NOTHROW(p.setVec4("tint_color",glm::vec4(1.0f)));
    }
    TEST_CASE("batch7778 missing vec3 uniform warning path remains safe") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag"); REQUIRE(p.ID!=0u); CHECK_NOTHROW(p.setVec3("batch7778_missing_vec3",glm::vec3(1.0f)));
    }
    TEST_CASE("batch7778 invalid vertex source reaches compile and link diagnostics") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Batch7366ShaderFiles files; REQUIRE(files.ready);
        files.write("bad.vert","#version 450 core\nthis is not valid glsl\n"); files.write("ok.frag","#version 450 core\nout vec4 c; void main(){c=vec4(1.0);}\n");
        Salix::OpenGLShaderProgram p(files.file("bad.vert").string(),files.file("ok.frag").string()); CHECK(p.ID!=0u);
    }
    TEST_CASE("batch7778 invalid fragment source reaches compile and link diagnostics") {
        Batch7778ShaderGLContext gl; REQUIRE(gl.ready); Batch7366ShaderFiles files; REQUIRE(files.ready);
        files.write("ok.vert","#version 450 core\nlayout(location=0) in vec3 p; void main(){gl_Position=vec4(p,1.0);}\n"); files.write("bad.frag","#version 450 core\nthis is not valid glsl\n");
        Salix::OpenGLShaderProgram p(files.file("ok.vert").string(),files.file("bad.frag").string()); CHECK(p.ID!=0u);
    }
}

TEST_SUITE("Salix::rendering::OpenGLShaderProgramBatch7878") {
    TEST_CASE("batch7878 missing mat4 uniform emits safe warning path") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setMat4("__missing_mat4_7878__",glm::mat4(1.0f))); }
    TEST_CASE("batch7878 missing integer uniform emits safe warning path") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setInt("__missing_int_7878__",7)); }
    TEST_CASE("batch7878 missing vec4 uniform emits safe warning path") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setVec4("__missing_vec4_7878__",glm::vec4(1.0f))); }
    TEST_CASE("batch7878 second missing mat4 name remains safe") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/color.vert","Assets/Shaders/OpenGL/2D/color.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setMat4("__missing_mat4_b_7878__",glm::mat4(2.0f))); }
    TEST_CASE("batch7878 second missing integer name remains safe") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/color.vert","Assets/Shaders/OpenGL/2D/color.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setInt("__missing_int_b_7878__",-7)); }
    TEST_CASE("batch7878 second missing vec4 name remains safe") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/color.vert","Assets/Shaders/OpenGL/2D/color.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setVec4("__missing_vec4_b_7878__",glm::vec4(0.5f))); }
    TEST_CASE("batch7878 missing vec3 uniform remains safe beside newly covered setters") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/color.vert","Assets/Shaders/OpenGL/2D/color.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setVec3("__missing_vec3_7878__",glm::vec3(1.0f))); }
    TEST_CASE("batch7878 valid model uniform still accepts nonidentity matrix") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setMat4("model",glm::mat4(2.0f))); }
    TEST_CASE("batch7878 valid sampler uniform accepts nonzero unit") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setInt("texture_sampler",1)); }
    TEST_CASE("batch7878 valid tint uniform accepts zero vector") { Batch7778ShaderGLContext gl;REQUIRE(gl.ready);Salix::OpenGLShaderProgram p("Assets/Shaders/OpenGL/2D/textured.vert","Assets/Shaders/OpenGL/2D/textured.frag");REQUIRE(p.ID);CHECK_NOTHROW(p.setVec4("tint_color",glm::vec4(0.0f))); }
}
