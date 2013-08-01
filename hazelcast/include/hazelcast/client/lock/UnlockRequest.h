//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_UnlockRequest
#define HAZELCAST_UnlockRequest

#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include "LockPortableHook.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace lock {
            class UnlockRequest {
            public:
            public:
                UnlockRequest(const serialization::Data& key, int threadId)
                :key(key)
                , threadId(threadId)
                , force(false) {
                };

                UnlockRequest(const serialization::Data& key, int threadId, bool force)
                :key(key)
                , threadId(threadId)
                , force(false) {
                };

                int getClassId() const {
                    return LockPortableHook::UNLOCK;
                };

                int getFactoryId() const {
                    return LockPortableHook::FACTORY_ID;
                };

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeInt("tid", threadId);
                    writer.writeBoolean("force", force);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    threadId = reader.readInt("tid");
                    force = reader.readBoolean("force");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
                };
            private:

                serialization::Data key;
                int threadId;
                bool force;
            };
        }
    }
}

#endif //HAZELCAST_UnlockRequest
