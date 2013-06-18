//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_ALL_REQUEST
#define HAZELCAST_MAP_PUT_ALL_REQUEST

#include "../impl/MapEntrySet.h"
#include "RequestIDs.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class PutAllRequest {
            public:
                PutAllRequest(const std::string& name, impl::MapEntrySet& entrySet)
                :name(name)
                , entrySet(entrySet) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return map::RequestIDs::F_ID;
                }

                int getClassId() const {
                    return map::RequestIDs::PUT_ALL;
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
                impl::MapEntrySet& entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST
