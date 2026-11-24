// Tests for Editor/GridSettings.
#include "fixtures/RealmFixtures.h"
#include <Editor/GridSettings.h>
#include <limits>
namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI003_GridSettings") {
    TEST_CASE("CRI003 GridSettings: default factory returns documented geometry") {
        auto s=GridSettings::Default();CHECK(s.size==20);CHECK(s.major_division==1);CHECK(s.minor_division==.25f);CHECK(s.snap_enabled);CHECK(s.snap_size==.25f);
    }

    TEST_CASE("CRI003 GridSettings: constructor clamps low and high size") {
        GridSettings low(-5),high(5000);CHECK(low.size==1);CHECK(high.size==1000);
    }

    TEST_CASE("CRI003 GridSettings: constructor clamps excessive minor division") {
        GridSettings s(20,2,9);CHECK(s.minor_division==1);
    }

    TEST_CASE("CRI003 GridSettings: constructor clamps snap upper bound") {
        GridSettings s(20,2,.5f,true,8);CHECK(s.snap_size==2);
    }

    TEST_CASE("CRI003 GridSettings: validate corrects too-small size and major division") {
        GridSettings s;s.size=-5;s.major_division=-3;s.validate();CHECK(s.size==1);CHECK(s.major_division==doctest::Approx(.1));
    }

    TEST_CASE("CRI003 GridSettings: validate clamps oversized size and minor division") {
        GridSettings s;s.size=1500;s.major_division=2;s.minor_division=9;s.validate();CHECK(s.size==1000);CHECK(s.minor_division==doctest::Approx(1.8));
    }

    TEST_CASE("CRI003 GridSettings: validate clamps negative minor and snap values") {
        GridSettings s;s.minor_division=-2;s.snap_size=-1;s.validate();CHECK(s.minor_division==doctest::Approx(.01));CHECK(s.snap_size==doctest::Approx(.01));
    }

    TEST_CASE("CRI003 GridSettings: validate limits snap to major spacing") {
        GridSettings s;s.major_division=3;s.snap_size=9;s.validate();CHECK(s.snap_size==3);
    }

    TEST_CASE("CRI003 GridSettings: validation is idempotent and preserves disabled snapping") {
        GridSettings s;s.snap_enabled=false;s.major_division=2;s.minor_division=4;s.validate();auto before=s;s.validate();CHECK(s.minor_division==before.minor_division);CHECK(s.snap_size==before.snap_size);CHECK_FALSE(s.snap_enabled);
    }

    TEST_CASE("CRI003 GridSettings: effective grid size depends on realm bounds") {
        GridSettings s(42);CHECK(s.get_effective_size(RealmSettings(true,100))==42);CHECK(s.get_effective_size(RealmSettings(false,100))==std::numeric_limits<float>::max());
    }
}
