// Tests for Editor/camera/EditorCamera.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../fixtures/ArchetypeFixtures.h"
#include <Editor/camera/EditorCamera.h>
#include <Salix/ecs/Transform.h>
#include <Salix/events/sdl/SDLEvent.h>
#include <Salix/core/InitContext.h>
#include <cmath>
#include "../fixtures/RealmFixtures.h"
#include <Salix/input/ImGuiInputManager.h>

namespace {
namespace coverage_batch_001 {
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
} // namespace coverage_batch_001
} // namespace

namespace {
namespace coverage_batch_003 {
namespace {
using namespace EditorCri;
struct CameraInput:ImGuiInputManager {
    bool right=false,middle=false,shift=false,w=false,s=false,a=false,d=false,space=false,c=false,forward=false,backward=false;
    bool is_held_down(MouseButton b)const override{return b==MouseButton::Right?right:b==MouseButton::Middle?middle:false;}
    bool is_held_down(KeyCode k)const override{switch(k){case KeyCode::LeftShift:return shift;case KeyCode::W:return w;case KeyCode::S:return s;case KeyCode::A:return a;case KeyCode::D:return d;case KeyCode::Space:return space;case KeyCode::C:return c;default:return false;}}
    bool did_scroll(MouseScroll direction)override{return direction==MouseScroll::Forward?forward:backward;}
};
struct CameraFixture {GuiFrame gui; CameraInput input; InitContext init{};EditorContext ctx;EditorCamera cam;
    CameraFixture(){init.input_manager=&input;ctx.init_context=&init;cam.initialize(&ctx);cam.set_mouse_inside_scene(true);cam.set_2D_mode(true);cam.get_transform()->set_position(0,0,0);}
    void step(float x=100,float y=100){ImGui::GetIO().AddMousePosEvent(x,y);gui.frame([&]{cam.on_update(.016f);});}
    glm::vec3 pos(){return cam.get_transform()->get_position().to_glm();}
};
}
TEST_SUITE("EditorCRI003_EditorCamera") {
    TEST_CASE("CRI003 EditorCamera: update before initialize preserves position") {
        EditorCamera c;c.get_transform()->set_position(1,2,3);c.set_mouse_inside_scene(true);c.on_update(.1f);CHECK(c.get_transform()->get_position().x==1);
    }

    TEST_CASE("CRI003 EditorCamera: missing input safely skips movement") {
        InitContext i{};EditorContext ctx;ctx.init_context=&i;EditorCamera c;c.initialize(&ctx);c.set_mouse_inside_scene(true);c.get_transform()->set_position(1,2,3);c.on_update(.1f);CHECK(c.get_transform()->get_position().y==2);
    }

    TEST_CASE("CRI003 EditorCamera: mouse outside scene ignores held movement and zoom") {
        CameraFixture f;f.cam.set_mouse_inside_scene(false);f.input.right=true;f.input.w=true;f.input.forward=true;auto before=f.pos();f.step();CHECK(glm::length(f.pos()-before)==doctest::Approx(0));
    }

    TEST_CASE("CRI003 EditorCamera: 2D movement follows up and down") {
        CameraFixture f;f.input.right=true;f.input.w=true;auto up=f.cam.get_transform()->get_up();f.step();CHECK(glm::dot(f.pos(),up)>0);f.input.w=false;f.input.s=true;f.step();CHECK(glm::length(f.pos())==doctest::Approx(0));
    }

    TEST_CASE("CRI003 EditorCamera: shift accelerates horizontal movement") {
        CameraFixture f;f.input.right=true;f.input.d=true;f.step();float normal=glm::length(f.pos());REQUIRE(normal>0);f.cam.get_transform()->set_position(0,0,0);f.input.shift=true;f.step();CHECK(glm::length(f.pos())==doctest::Approx(normal*3));
    }

    TEST_CASE("CRI003 EditorCamera: 3D forward and reverse movement cancel") {
        CameraFixture f;f.cam.set_2D_mode(false);f.input.right=true;f.input.w=true;f.step();CHECK(glm::length(f.pos())>0);f.input.w=false;f.input.s=true;f.step();CHECK(glm::length(f.pos())==doctest::Approx(0).epsilon(.001));
    }

    TEST_CASE("CRI003 EditorCamera: perspective wheel moves camera then reverses") {
        CameraFixture f;f.input.forward=true;f.step();float d=glm::length(f.pos());CHECK(d==doctest::Approx(.2));f.input.forward=false;f.input.backward=true;f.step();CHECK(glm::length(f.pos())==doctest::Approx(0));
    }

    TEST_CASE("CRI003 EditorCamera: orthographic wheel changes projection without translation") {
        CameraFixture f;f.cam.set_projection_mode(ProjectionMode::Orthographic);f.cam.set_orthographic_size(10);auto before=f.cam.get_projection_matrix()[1][1];f.input.forward=true;f.step();CHECK(f.cam.get_projection_matrix()[1][1]==doctest::Approx(before/.9f));CHECK(glm::length(f.pos())==doctest::Approx(0));f.input.forward=false;f.input.backward=true;f.step();CHECK(f.cam.get_projection_matrix()[1][1]==doctest::Approx(before/(.9f*1.1f)));
    }

    TEST_CASE("CRI003 EditorCamera: middle mouse first event establishes origin then pans") {
        CameraFixture f;f.input.middle=true;f.step(100,100);CHECK(glm::length(f.pos())==doctest::Approx(0));f.step(120,100);CHECK(glm::length(f.pos())==doctest::Approx(1));f.input.middle=false;f.step(300,300);auto p=f.pos();f.input.middle=true;f.step(500,500);CHECK(glm::length(f.pos()-p)==doctest::Approx(0));
    }

    TEST_CASE("CRI003 EditorCamera: focus overloads place camera at requested distance") {
        EditorCamera c;Transform t;t.set_position(3,4,5);auto target=t.get_world_position().to_glm();c.focus_on(&t,4);CHECK(glm::length(c.get_transform()->get_position().to_glm()-target)==doctest::Approx(4));c.simple_focus_on(&t,2);CHECK(glm::length(c.get_transform()->get_position().to_glm()-target)==doctest::Approx(2));auto p=c.get_transform()->get_position();c.focus_on(nullptr,2);c.simple_focus_on(nullptr,2);CHECK(c.get_transform()->get_position()==p);
    }
}
} // namespace coverage_batch_003
} // namespace
