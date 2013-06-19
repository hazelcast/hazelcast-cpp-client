//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_TRY_PUT_REQUEST
#define HAZELCAST_MAP_TRY_PUT_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class TryPutRequest {
            public:
                TryPutRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long timeout)
                :name(name)
                , key(key)
                , value(value)
                , threadId(threadId)
                , ttl(-1)
                , timeout(timeout) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::TRY_PUT;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["timeout"] << timeout;
                    writer["n"] << name;
                    writer["t"] << threadId;
                    writer["ttl"] << ttl;
                    writer << key;
                    writer << value;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["timeout"] >> timeout;
                    reader["n"] >> name;
                    reader["t"] >> threadId;
                    reader["ttl"] >> ttl;
                    reader >> key;
                    reader >> value;
                };
            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
                long ttl;
                long timeout;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
