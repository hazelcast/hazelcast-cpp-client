//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST
#define HAZELCAST_MAP_PUT_IF_ABSENT_REQUEST

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API PutIfAbsentRequest : public impl::PortableRequest {
            public:
                PutIfAbsentRequest(const std::string& name, serialization::Data& key, serialization::Data& value, int threadId, long ttl)
                :name(name)
                , key(key)
                , value(value)
                , threadId(threadId)
                , ttl(ttl) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::PUT_IF_ABSENT;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("t", threadId);
                    writer.writeLong("ttl", ttl);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    value.writeData(out);
                };

            private:
                serialization::Data& key;
                serialization::Data& value;
                std::string name;
                int threadId;
                long ttl;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
