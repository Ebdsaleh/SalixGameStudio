// Salix/ecs/CppScript.cpp
#include <Salix/ecs/CppScript.h>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
CEREAL_REGISTER_TYPE(Salix::ScriptElement)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Salix::Element, Salix::ScriptElement)
namespace Salix {

    const std::string& CppScript::get_script_name() const {
        return script_name;
    }

    template<class Archive>
    void CppScript::serialize(Archive& archive) {
        archive( cereal::base_class<ScriptElement>(this) );
        archive( cereal::make_nvp("script_name", script_name) );
    }

    // Add these explicit instantiations for the serialize function
    template void Salix::CppScript::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&);
    template void Salix::CppScript::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void Salix::CppScript::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);
    template void Salix::CppScript::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);
} // namespace Salix