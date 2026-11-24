// Tests for Editor/RealmSettings.
#include "fixtures/RealmFixtures.h"
#include <Editor/RealmSettings.h>
namespace {
using namespace EditorCri;

}
TEST_SUITE("EditorCRI003_RealmSettings") {
    TEST_CASE("CRI003 RealmSettings: default factory enables bounds at documented size") {
        auto r=RealmSettings::Default();CHECK(r.use_realm_bounds);CHECK(r.realm_size==1000);
    }

    TEST_CASE("CRI003 RealmSettings: constructor clamps negative and zero realm sizes") {
        CHECK(RealmSettings(true,-3).realm_size==1);CHECK(RealmSettings(true,0).realm_size==1);
    }

    TEST_CASE("CRI003 RealmSettings: disabled bounds still retain configured size") {
        RealmSettings r(false,42);CHECK_FALSE(r.use_realm_bounds);CHECK(r.realm_size==42);
    }

    TEST_CASE("CRI003 RealmSettings: runtime validation clamps edited negative size") {
        RealmSettings r;r.realm_size=-9;r.validate();CHECK(r.realm_size==1);
    }

    TEST_CASE("CRI003 RealmSettings: runtime validation preserves valid large size") {
        RealmSettings r(false,4500);r.validate();CHECK(r.realm_size==4500);CHECK_FALSE(r.use_realm_bounds);
    }

    TEST_CASE("CRI003 RealmSettings: identical values compare equal") {
        RealmSettings a(true,20),b(true,20);CHECK(a==b);CHECK_FALSE(a!=b);
    }

    TEST_CASE("CRI003 RealmSettings: bounds toggle changes equality independent of size") {
        RealmSettings a(true,20),b(false,20);CHECK(a!=b);CHECK_FALSE(a==b);
    }

    TEST_CASE("CRI003 RealmSettings: near equal sizes compare symmetrically") {
        RealmSettings a(true,2),b(true,2.00005f);CHECK(a==b);CHECK(b==a);
    }

    TEST_CASE("CRI003 RealmSettings: larger size difference compares unequal symmetrically") {
        RealmSettings a(false,2),b(false,2.001f);CHECK(a!=b);CHECK(b!=a);
    }

    TEST_CASE("CRI003 RealmSettings: validate normalizes edited and constructed invalid size consistently") {
        RealmSettings a(true,-5),b;b.realm_size=0;b.validate();CHECK(a==b);CHECK(b.realm_size==1);
    }
}
