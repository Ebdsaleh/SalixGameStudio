// CRI relay 001: EditorCamera; actual Editor DLL APIs, no native windows.
#include "CriRelay001Fixtures.h"
#include <Editor/camera/EditorCamera.h>
#include <Salix/ecs/Transform.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/core/InitContext.h>
#include <cmath>
namespace {
using namespace EditorCri;
bool finite_matrix(const glm::mat4& m){for(int c=0;c<4;++c)for(int r=0;r<4;++r)if(!std::isfinite(m[c][r]))return false;return true;}
}
TEST_SUITE("EditorCRI001_EditorCamera") {
    TEST_CASE("CRI001 EditorCamera: default clip planes and perspective") {
        EditorCamera c; CHECK(c.get_near_plane()==doctest::Approx(.1)); CHECK(c.get_far_plane()==doctest::Approx(1000)); CHECK(c.get_projection_mode()==ProjectionMode::Perspective); CHECK(finite_matrix(c.get_projection_matrix()));
    }

    TEST_CASE("CRI001 EditorCamera: near plane update invalidates projection") {
        EditorCamera c; auto a=c.get_projection_matrix(); c.set_near_plane(2); CHECK(c.get_near_plane()==2); CHECK(c.get_projection_matrix()[3][2]!=a[3][2]);
    }

    TEST_CASE("CRI001 EditorCamera: far plane update and invalid near rejected") {
        EditorCamera c; c.set_far_plane(50); c.set_near_plane(50); CHECK(c.get_far_plane()==50); CHECK(c.get_near_plane()==doctest::Approx(.1));
    }

    TEST_CASE("CRI001 EditorCamera: far plane below near rejected") {
        EditorCamera c; c.set_near_plane(2); c.set_far_plane(1); CHECK(c.get_far_plane()==1000); CHECK(c.get_near_plane()==2);
    }

    TEST_CASE("CRI001 EditorCamera: orthographic mode and size alter matrix") {
        EditorCamera c; c.set_projection_mode(ProjectionMode::Orthographic); c.set_viewport_size(100,100); c.set_orthographic_size(10); CHECK(c.get_projection_mode()==ProjectionMode::Orthographic); CHECK(c.get_projection_matrix()[1][1]==doctest::Approx(.2)); c.set_orthographic_size(20); CHECK(c.get_projection_matrix()[1][1]==doctest::Approx(.1));
    }

    TEST_CASE("CRI001 EditorCamera: viewport zero height preserves prior projection") {
        EditorCamera c; c.set_viewport_size(100,100); auto a=c.get_projection_matrix(); c.set_viewport_size(200,0); CHECK(c.get_projection_matrix()[0][0]==a[0][0]); c.set_viewport_size(200,100); CHECK(c.get_projection_matrix()[0][0]==doctest::Approx(a[0][0]/2));
    }

    TEST_CASE("CRI001 EditorCamera: resize event updates aspect ratio") {
        EditorCamera c; c.set_viewport_size(100,100); auto a=c.get_projection_matrix(); WindowResizeEvent e(300,100); c.on_event(e); CHECK(c.get_projection_matrix()[0][0]==doctest::Approx(a[0][0]/3));
    }

    TEST_CASE("CRI001 EditorCamera: activation transitions are idempotent") {
        EditorCamera c; CHECK(c.get_is_active()); c.deactivate(); c.deactivate(); CHECK_FALSE(c.get_is_active()); c.activate(); c.activate(); CHECK(c.get_is_active()); c.set_is_active(false); CHECK_FALSE(c.get_is_active());
    }

    TEST_CASE("CRI001 EditorCamera: bounds clamp position and warning direction") {
        InitContext init{}; EditorContext ctx; ctx.init_context=&init; ctx.realm_settings.use_realm_bounds=true; ctx.realm_settings.realm_size=10; EditorCamera c; c.initialize(&ctx); c.get_transform()->set_position(20,2,-20); c.check_scene_bounds(); CHECK(c.get_should_draw_boundary_warning()); CHECK(c.get_boundary_violation_direction().x==1); CHECK(c.get_boundary_violation_direction().z==-1); CHECK(c.get_transform()->get_position().x==doctest::Approx(9)); CHECK(c.get_transform()->get_position().z==doctest::Approx(-9)); CHECK(c.get_transform()->get_position().y==2); c.check_scene_bounds(); CHECK_FALSE(c.get_should_draw_boundary_warning());
    }

    TEST_CASE("CRI001 EditorCamera: focus positions camera one unit from target") {
        EditorCamera c; c.get_transform()->set_position(0,0,0); c.focus_on(glm::vec3(0,0,10)); CHECK(c.get_transform()->get_position().z==doctest::Approx(9)); CHECK(finite_matrix(c.get_view_matrix())); c.focus_on(c.get_transform()->get_position().to_glm()); CHECK(c.get_transform()->get_position().z==doctest::Approx(9));
    }
}
