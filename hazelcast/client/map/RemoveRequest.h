//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REMOVE_REQUEST
#define HAZELCAST_MAP_REMOVE_REQUEST

#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class RemoveRequest : public hazelcast::client::serialization::Portable {
            public:
                RemoveRequest();

                RemoveRequest(const std::string& name, const hazelcast::client::serialization::Data& key, int threadId);

                hazelcast::client::serialization::Data key;
                std::string name;
                int threadId;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::map::RemoveRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_MAP_FACTORY_ID;
            }

            inline int getClassId(const hazelcast::client::map::RemoveRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::MAP_REMOVE_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::map::RemoveRequest& arr) {
                writer["n"] << arr.name;
                writer["t"] << arr.threadId;
                writer << arr.key;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::map::RemoveRequest& arr) {
                reader["n"] >> arr.name;
                reader["t"] >> arr.threadId;
                reader >> arr.key;
            };

        }
    }
}


#endif //HAZELCAST_MAP_REMOVE_REQUEST
