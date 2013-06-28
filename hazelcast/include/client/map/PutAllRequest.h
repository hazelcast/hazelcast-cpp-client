//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_ALL_REQUEST
#define HAZELCAST_MAP_PUT_ALL_REQUEST

#include "../map/MapEntrySet.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class PutAllRequest {
            public:
                PutAllRequest(const std::string& name, map::MapEntrySet& entrySet)
                :name(name)
                , entrySet(entrySet) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::PUT_ALL;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer << entrySet;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader >> entrySet;
                };
            private:
                std::string name;
                map::MapEntrySet& entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST
