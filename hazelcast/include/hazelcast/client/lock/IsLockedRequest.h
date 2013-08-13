//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_IsLockedRequest
#define HAZELCAST_IsLockedRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "LockPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace lock {
            class IsLockedRequest {
            public:
                IsLockedRequest(const serialization::Data& key)
                :key(key) {
                };

                IsLockedRequest(const serialization::Data& key, int threadId)
                :key(key)
                , threadId(threadId) {
                };

                int getClassId() const {
                    return LockPortableHook::IS_LOCKED;
                };

                int getFactoryId() const {
                    return LockPortableHook::FACTORY_ID;
                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeInt("tid", threadId);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    threadId = reader.readInt("tid");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                };
            private:
                int threadId;
                serialization::Data key;
            };
        }
    }
}


#endif //HAZELCAST_IsLockedRequest
