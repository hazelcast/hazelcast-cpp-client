//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_MAP_EVICT_REQUEST
#define HAZELCAST_MAP_EVICT_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class EvictRequest : public Portable {
            public:
                EvictRequest(const std::string& name, serialization::Data& key, int threadId)
                :name(name)
                , key(key)
                , threadId(threadId) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                };

                int getClassId() const {
                    return PortableHook::EVICT;
                };


                inline void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("name", name);
                    writer.writeInt("t", threadId);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                };


                inline void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("name");
                    threadId = reader.readInt("t");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                };
            private:
                serialization::Data& key;
                std::string name;
                int threadId;
            };
        }
    }
}

#endif //HAZELCAST_MAP_EVICT_REQUEST
