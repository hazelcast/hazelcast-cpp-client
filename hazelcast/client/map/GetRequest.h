//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_REQUEST
#define HAZELCAST_MAP_GET_REQUEST

#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class GetRequest {
            public:
                GetRequest();

                GetRequest(const std::string& name, const hazelcast::client::serialization::Data& key);

                hazelcast::client::serialization::Data key;
                std::string name;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::map::GetRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_MAP_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::map::GetRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::MAP_GET_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::map::GetRequest& arr) {
                writer["n"] << arr.name;
                writer << arr.key;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::map::GetRequest& arr) {
                reader["n"] >> arr.name;
                reader >> arr.key;
            };

        }
    }
}


#endif //HAZELCAST_MAP_GET_REQUEST
