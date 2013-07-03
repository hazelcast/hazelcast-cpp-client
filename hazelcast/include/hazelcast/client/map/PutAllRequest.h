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

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::PUT_ALL;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::BufferedDataOutput *out = writer.getRawDataOutput();
                    entrySet.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    serialization::BufferedDataInput *in = reader.getRawDataInput();
                    entrySet.readData(*in);
                };
            private:
                std::string name;
                map::MapEntrySet& entrySet;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST
