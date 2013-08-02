//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_IS_LOCKED_REQUEST
#define HAZELCAST_MAP_IS_LOCKED_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include "../serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class IsLockedRequest : public Portable {
            public:
                IsLockedRequest(const std::string& name, serialization::Data& key)
                :key(key),
                name(name) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::IS_LOCKED;
                }


                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("name", name);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("name");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
                };
            private:
                std::string name;
                serialization::Data& key;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST
