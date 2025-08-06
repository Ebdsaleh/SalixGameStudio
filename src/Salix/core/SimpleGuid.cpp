// =================================================================================
// Filename:    Salix/core/SimpleGuid.cpp
// Author:      SalixGameStudio
// Description: Implements the SimpleGuid class.
// =================================================================================
#include <Salix/core/SimpleGuid.h>
#include <yaml-cpp/yaml.h>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <Salix/core/SerializationRegistrations.h>
#include <cereal/cereal.hpp>




namespace Salix {

    // --- The static counter lives here, safely hidden in the .cpp file. ---
    // It is initialized to 1, ensuring no object gets the invalid ID of 0.
    static uint64_t next_id_counter = 1;

    // --- Method Definitions ---

    SimpleGuid SimpleGuid::generate() {
        // This function is the only way to get a new ID. It increments the
        // hidden counter and returns a new SimpleGuid object.
        return SimpleGuid(next_id_counter++);
    }

    SimpleGuid SimpleGuid::invalid() {
        return SimpleGuid();
    }

    // --- Constructor Implementations ---
    SimpleGuid::SimpleGuid() : id(0) {}
    SimpleGuid::SimpleGuid(uint64_t guid) : id(guid) {}



    // --- Operator Implementations ---
    bool SimpleGuid::operator==(const SimpleGuid& other) const { return id == other.id; }
    bool SimpleGuid::operator!=(const SimpleGuid& other) const { return id != other.id; }
    bool SimpleGuid::operator<(const SimpleGuid& other) const { return id < other.id; }

    template<class Archive>
    void SimpleGuid::serialize(Archive& archive) {
            archive( cereal::make_nvp("id", id) );
        }
    
    template void SimpleGuid::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);
    template void SimpleGuid::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);
    template void SimpleGuid::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive &);
    template void SimpleGuid::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive &);


} // namespace Salix

