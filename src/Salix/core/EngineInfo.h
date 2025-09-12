// EngineInfo.h
#pragma once
#include <Salix/core/Core.h>
#include <string>
namespace Salix {
    struct SALIX_API EngineInfo{ 
        static const std::string version_id;
        static const std::string& get_engine_version();
        
    };
}