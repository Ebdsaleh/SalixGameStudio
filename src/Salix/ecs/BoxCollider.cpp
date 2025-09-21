// Salix/ecs/BoxCollider.cpp
#include <Salix/ecs/BoxCollider.h>
#include <Salix/core/InitContext.h>
#include <Salix/math/Vector3.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <Salix/core/SerializationRegistrations.h>
#include <iostream>
#include <memory>


namespace Salix {

    struct BoxCollider::Pimpl { 
        Vector3 size = Vector3(1.0f, 1.0f, 1.0f);
    };

    BoxCollider::BoxCollider() :pimpl(std::make_unique<Pimpl>()) {
        set_name(get_class_name());
    }
    BoxCollider::~BoxCollider() = default;

    void BoxCollider::initialize() {}

    void BoxCollider::on_load(const InitContext& context) {
        if (!context.renderer) return;

    }


    void BoxCollider::update(float delta_time) {
        (void) delta_time;
    }


    const Vector3& BoxCollider::get_size() const {
        return pimpl->size;
    }
    
    void BoxCollider::set_size(const Vector3& new_size) {
        pimpl->size = new_size;
    }

    template <class Archive>
    void BoxCollider::serialize(Archive& archive) {
        // 1. First, tell Cereal to serialize the data from the base class (Element).
        archive(cereal::base_class<Element>(this));

        archive(cereal::make_nvp("size", pimpl->size));
    }

    template void SALIX_API BoxCollider::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&);
    template void SALIX_API BoxCollider::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&);
    template void SALIX_API BoxCollider::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);
    template void SALIX_API BoxCollider::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);
}