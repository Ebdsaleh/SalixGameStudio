// =================================================================================
// Filename:    Salix/core/SimpleGuid.h
// Author:      SalixGameStudio
// Description: Defines a Simple, Unique Identifier for use within the engine.
// =================================================================================
#pragma once
#include <Salix/core/Core.h>
#include <cstdint>
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

namespace Salix {

    // A type-safe wrapper around a 64-bit unsigned integer.
    class SALIX_API SimpleGuid {
    public:
        // Public method to get a new, unique ID. This is the only way to create one.
        static SimpleGuid generate();

        // Constructors are made private to ensure IDs are only made via generate().
    private:
        SimpleGuid(); // Default constructor creates an invalid ID (0)
        explicit SimpleGuid(uint64_t guid);

    public: // Public operators for comparison and use in maps.
        bool operator==(const SimpleGuid& other) const;
        bool operator!=(const SimpleGuid& other) const;
        bool operator<(const SimpleGuid& other) const;

        
    private:
        friend class cereal::access;
        // Cereal needs access to the private 'id' member to save/load it.
        template<class Archive>
        void serialize(Archive& archive);

        uint64_t id;
    };
}