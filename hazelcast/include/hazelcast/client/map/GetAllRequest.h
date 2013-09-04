//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_GET_ALL_REQUEST
#define HAZELCAST_MAP_GET_ALL_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class GetAllRequest : public Portable, public RetryableRequest {
            public:
                GetAllRequest(const std::string& name, std::vector<serialization::Data>& keys)
                :name(name)
                , keys(keys) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::GET_ALL;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeInt("size", keys.size());
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    for (int i = 0; i < keys.size(); i++) {
                        keys[i].writeData(out);
                    }
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    int size = reader.readInt("size");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    keys.resize(size);
                    for (int i = 0; i < keys.size(); i++) {
                        keys[i].readData(in);
                    }
                };
            private:
                std::vector<serialization::Data> keys;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_GET_ALL_REQUEST
