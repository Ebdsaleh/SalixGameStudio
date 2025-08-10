// Editor/management/RealmLoader.h

#pragma once
#include <Editor/EditorAPI.h>
#include <Editor/Archetypes.h>
#include <vector>
#include <string>

namespace Salix {

    // This is the function DECLARATION.
    // It tells the compiler "this function exists somewhere."
    std::vector<EntityArchetype> load_archetypes_from_file(const std::string& filepath);

} // namespace Salix