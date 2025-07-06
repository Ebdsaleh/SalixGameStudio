// Salix/core/IPlugin.h
#pragma once
#include <Salix/core/Core.h>
#include <Salix/core/InitContext.h>

namespace Salix{
    struct SALIX_API IPlugin {
        virtual void on_register_types() = 0;
        virtual void on_startup(InitContext&) = 0;
        virtual void on_shutdown() = 0;
        virtual ~IPlugin() = default;
    };
}