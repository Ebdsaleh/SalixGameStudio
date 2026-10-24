#include <doctest.h>
#include <Salix/rendering/opengl/OpenGLShaderProgram.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <type_traits>
#include <utility>

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
