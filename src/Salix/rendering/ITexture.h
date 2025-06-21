// ITexture.h
#pragma once

#include <Salix/core/Core.h>
namespace Salix {

    class SALIX_API ITexture {
        public:
            virtual ~ITexture() = default;

            // A contract that all textures must be able to report their dimensions.
            virtual int get_width() const = 0;
            virtual int get_height() const = 0;

    };
}