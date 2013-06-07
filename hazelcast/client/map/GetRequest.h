//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_REQUEST
#define HAZELCAST_MAP_GET_REQUEST

#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class GetRequest {
            public:
                GetRequest(const std::string& name, serialization::Data& key);

                serialization::Data& key;
                std::string name;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getTypeSerializerId(const map::GetRequest& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const map::GetRequest& ar) {
                return protocol::ProtocolConstants::CLIENT_MAP_FACTORY_ID;
            }

            inline int getClassId(const map::GetRequest& ar) {
                return protocol::ProtocolConstants::MAP_GET_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const map::GetRequest& arr) {
                writer["n"] << arr.name;
                writer << arr.key;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, map::GetRequest& arr) {
                reader["n"] >> arr.name;
                reader >> arr.key;
            };

        }
    }
}


#endif //HAZELCAST_MAP_GET_REQUEST
