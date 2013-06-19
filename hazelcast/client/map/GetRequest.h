//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_REQUEST
#define HAZELCAST_MAP_GET_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class GetRequest {
            public:
                GetRequest(std::string& name, serialization::Data& key)
                :name(name)
                , key(key) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::GET;
                }


                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["n"] << name;
                    writer << key;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["n"] >> name;
                    reader >> key;
                };
            private:
                serialization::Data& key;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_REQUEST
