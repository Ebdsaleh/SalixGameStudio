// Salix/ecs/CppScript.cpp
#include <Salix/ecs/CppScript.h>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <Salix/core/SerializationRegistrations.h>

namespace Salix {

    const std::string& CppScript::get_script_name() const {
        return script_name;
    }

    template<class Archive>
    void CppScript::serialize(Archive& archive) {
        archive( cereal::base_class<ScriptElement>(this),
        cereal::make_nvp("script_name", script_name) );
    }

    template void Salix::CppScript::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&);
    template void Salix::CppScript::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void Salix::CppScript::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);
    template void Salix::CppScript::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);
} // namespace Salix