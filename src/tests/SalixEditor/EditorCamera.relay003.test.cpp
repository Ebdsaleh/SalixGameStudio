// CRI relay 003: EditorCamera; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Editor/camera/EditorCamera.h>
#include <Salix/input/ImGuiInputManager.h>
#include <Salix/events/sdl/SDLEvent.h>
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
