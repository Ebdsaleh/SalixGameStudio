// Editor/panels/WorldTreeNode.h
#pragma once
#include <Editor/EditorAPI.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <Editor/Archetypes.h>
#include <Salix/core/SimpleGuid.h>

namespace Salix {
    struct EDITOR_API WorldTreeNode{
            SimpleGuid entity_id;
            std::vector<std::shared_ptr<WorldTreeNode>> children;

            WorldTreeNode() : entity_id(SimpleGuid::invalid()) {}

            // Recursive self-print method
            void print(int depth = 0) const {
                for (int i = 0; i < depth; ++i) std::cout << "\t";
                std::cout << entity_id.get_value() << "\n";
                for (const auto& child : children) {
                    if (child) child->print(depth + 1);
                }
            }
            // A function that prints elements
            void WorldTreeNode::print_elements_shallow(const std::vector<ElementArchetype>& elements, const std::string& indent) const {
                for (const auto& elem : elements) {
                    std::cout << indent << "* " << elem.type_name << " (ID: " << elem.id.get_value() << ")\n";
                }
            }
        };
}