// CRI relay 003: WorldTreeNode; uncovered paths and behavioral regressions.
#include "CriRelay003Fixtures.h"
#include <Editor/panels/WorldTreeNode.h>
namespace {
using namespace EditorCri;
std::shared_ptr<WorldTreeNode> node(uint64_t n){auto p=std::make_shared<WorldTreeNode>();p->entity_id=id(n);return p;}
}
TEST_SUITE("EditorCRI003_WorldTreeNode") {
    TEST_CASE("CRI003 WorldTreeNode: new node has invalid ID and no children") {
        WorldTreeNode n;CHECK_FALSE(n.entity_id.is_valid());CHECK(n.children.empty());
    }

    TEST_CASE("CRI003 WorldTreeNode: leaf prints stable identity") {
        auto n=node(11);CoutCapture out;n->print();CHECK(out.str()=="11\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: explicit depth controls indentation") {
        auto n=node(11);CoutCapture out;n->print(2);CHECK(out.str()=="\t\t11\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: recursive print preserves child order") {
        auto n=node(11);n->children={node(12),node(13)};CoutCapture out;n->print();CHECK(out.str()=="11\n\t12\n\t13\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: three levels render increasing depth") {
        auto n=node(11);auto c=node(12);c->children={node(13)};n->children={c};CoutCapture out;n->print();CHECK(out.str()=="11\n\t12\n\t\t13\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: null child skipped without dropping sibling") {
        auto n=node(11);n->children={nullptr,node(12),nullptr};CoutCapture out;n->print();CHECK(out.str()=="11\n\t12\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: empty element list writes no output") {
        WorldTreeNode n;CoutCapture out;n.print_elements_shallow({},"  ");CHECK(out.str().empty());
    }

    TEST_CASE("CRI003 WorldTreeNode: element print shows type and stable ID") {
        WorldTreeNode n;CoutCapture out;n.print_elements_shallow({element("Transform",201)},"  ");CHECK(out.str()=="  * Transform (ID: 201)\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: multiple elements printed in vector order") {
        WorldTreeNode n;CoutCapture out;n.print_elements_shallow({element("BoxCollider",202),element("Transform",201)},"-");CHECK(out.str()=="-* BoxCollider (ID: 202)\n-* Transform (ID: 201)\n");
    }

    TEST_CASE("CRI003 WorldTreeNode: printing leaves hierarchy and identities unchanged") {
        auto n=node(11);auto c=node(12);n->children={c};CoutCapture out;n->print();n->print();CHECK(out.str()=="11\n\t12\n11\n\t12\n");CHECK(n->children.size()==1);CHECK(n->children[0]==c);CHECK(c->entity_id==id(12));
    }
}
