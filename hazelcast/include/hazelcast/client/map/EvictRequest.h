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

                int getFactoryId() {
                    return PortableHook::F_ID;
                };

                int getClassId() {
                    return PortableHook::EVICT;
                };

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer.writeUTF("name", name);
                    writer.writeInt("t", threadId);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    name = reader.readUTF("name");
                    threadId = reader.readInt("t");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
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
