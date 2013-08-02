//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_CONTAINS_VALUE_R
#define HAZELCAST_MAP_CONTAINS_VALUE_R

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class ContainsValueRequest : public Portable {
            public:
                ContainsValueRequest(const std::string& name, serialization::Data& value)
                :name(name)
                , value(value) {
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::CONTAINS_VALUE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    value.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    value.readData(*in);
                };
            private:
                serialization::Data& value;
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
