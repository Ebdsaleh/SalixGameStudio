// Salix/ecs/BoxCollider.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/ecs/Element.h>
#include <Salix/math/Vector3.h>
#include <memory>

namespace Salix {

    struct InitContext;
    class SALIX_API BoxCollider : public Element {
    public:


        BoxCollider();
        ~BoxCollider() override;
        const char* get_class_name() const override;
        void initialize() override;
        void on_load(const InitContext& context) override;
        void update(float delta_time) override;
        // Required for ByteMirror.  
        const Vector3& get_size() const;
        void set_size(const Vector3& new_size);
       
    private:
        struct Pimpl;
        std::unique_ptr<Pimpl> pimpl;
    };

}