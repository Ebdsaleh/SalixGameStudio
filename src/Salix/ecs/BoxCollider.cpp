// Salix/ecs/BoxCollider.cpp
#include <Salix/ecs/BoxCollider.h>
#include <Salix/core/InitContext.h>
#include <Salix/math/Vector3.h>
#include <iostream>
#include <memory>

namespace Salix {

    struct BoxCollider::Pimpl { 
        Vector3 size = Vector3(1.0f, 1.0f, 1.0f);
        const char* class_name = "BoxCollider";
    };

    BoxCollider::BoxCollider() :pimpl(std::make_unique<Pimpl>()) {}
    BoxCollider::~BoxCollider() = default;

    void BoxCollider::initialize() {}

    void BoxCollider::on_load(const InitContext& context) {
        if (!context.renderer) return;

    }


    void BoxCollider::update(float delta_time) {
        (void) delta_time;
    }


    const char* BoxCollider::get_class_name() const {
        return pimpl->class_name;
    }



    const Vector3& BoxCollider::get_size() const {
        return pimpl->size;
    }
    
    void BoxCollider::set_size(const Vector3& new_size) {
        pimpl->size = new_size;
    }

}