// EngineInfo.h
#pragma once
#include <string>
namespace Salix {
    struct EngineInfo{ 
        static const std::string version_id;
        static const std::string& get_engine_version();
        
    };
}