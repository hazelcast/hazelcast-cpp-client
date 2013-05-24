//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_REQUEST
#define HAZELCAST_MAP_PUT_REQUEST

#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class PutRequest : public hazelcast::client::serialization::Portable {
            public:
                PutRequest();

                PutRequest(const std::string& name, const hazelcast::client::serialization::Data& key, const hazelcast::client::serialization::Data& value, int threadId, long ttl);

                hazelcast::client::serialization::Data key;
                hazelcast::client::serialization::Data value;
                std::string name;
                int threadId;
                long ttl;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::map::PutRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_MAP_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::map::PutRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::MAP_PUT_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::map::PutRequest& arr) {
                writer["n"] << arr.name;
                writer["t"] << arr.threadId;
                writer["ttl"] << arr.ttl;
                writer << arr.key;
                writer << arr.value;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::map::PutRequest& arr) {
                reader["n"] >> arr.name;
                reader["t"] >> arr.threadId;
                reader["ttl"] >> arr.ttl;
                reader >> arr.key;
                reader >> arr.value;
            };

        }
    }
}


#endif //HAZELCAST_MAP_PUT_REQUEST
