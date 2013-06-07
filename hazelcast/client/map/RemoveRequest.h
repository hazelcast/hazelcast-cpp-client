//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_REMOVE_REQUEST
#define HAZELCAST_MAP_REMOVE_REQUEST

#include "../serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class RemoveRequest {
            public:
                RemoveRequest(const std::string& name, serialization::Data& key, int threadId);

                serialization::Data& key;
                std::string name;
                int threadId;
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getTypeSerializerId(const map::RemoveRequest& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const map::RemoveRequest& ar) {
                return protocol::ProtocolConstants::CLIENT_MAP_FACTORY_ID;
            }

            inline int getClassId(const map::RemoveRequest& ar) {
                return protocol::ProtocolConstants::MAP_REMOVE_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const map::RemoveRequest& arr) {
                writer["n"] << arr.name;
                writer["t"] << arr.threadId;
                writer << arr.key;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, map::RemoveRequest& arr) {
                reader["n"] >> arr.name;
                reader["t"] >> arr.threadId;
                reader >> arr.key;
            };

        }
    }
}


#endif //HAZELCAST_MAP_REMOVE_REQUEST
