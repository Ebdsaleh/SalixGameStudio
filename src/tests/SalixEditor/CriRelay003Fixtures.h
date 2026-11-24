#pragma once
#include "CriRelay002Fixtures.h"
#include <Salix/reflection/EnumRegistry.h>
#include <Salix/serialization/YamlConverters.h>
#include <Salix/ecs/Entity.h>
#include <Salix/ecs/Transform.h>
#include <sstream>
#include <iostream>
namespace EditorCri {
    struct CoutCapture {
        std::ostringstream output;
        std::streambuf* previous;
        CoutCapture():previous(std::cout.rdbuf(output.rdbuf())){}
        ~CoutCapture(){std::cout.rdbuf(previous);}
        CoutCapture(const CoutCapture&)=delete;
        std::string str()const{return output.str();}
    };
    struct FamilyFixture {
        PanelContext services;
        EditorRealmManager& manager=*services.ctx.editor_realm_manager;
        FamilyFixture(){
            EnumRegistry::register_all_enums();
            SimpleGuid::update_next_id(100000);
            services.ctx.preview_realm=std::make_unique<Realm>("Preview");
        }
        void root(uint64_t n,const char* name){manager.add_entity(entity(n,name));}
        void child(uint64_t n,uint64_t p,const char* name){
            auto a=entity(n,name);a.parent_id=id(p);manager.add_child_entity(a);
        }
        void family(){root(11,"Root");child(12,11,"Child");child(13,12,"Grandchild");root(14,"Other");manager.take_snapshot();}
        EntityArchetype* get(uint64_t n){return manager.get_archetype(id(n));}
    };
}
