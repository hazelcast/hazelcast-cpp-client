//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_PUT_ALL_REQUEST
#define HAZELCAST_MAP_PUT_ALL_REQUEST

#include "../impl/MapEntrySet.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class PutAllRequest : public Portable {
            public:
                PutAllRequest(const std::string& name, map::MapEntrySet& entrySet)
                :name(name)
                , entrySet(entrySet) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::PUT_ALL;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    entrySet.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    entrySet.readData(in);
                };
            private:
                std::string name;
                map::MapEntrySet& entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST
