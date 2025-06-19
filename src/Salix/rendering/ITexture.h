// ITexture.h
#pragma once

namespace Salix {

    class ITexture {
        public:
            virtual ~ITexture() = default;

            // A contract that all textures must be able to report their dimensions.
            virtual int get_width() const = 0;
            virtual int get_height() const = 0;

    };
}