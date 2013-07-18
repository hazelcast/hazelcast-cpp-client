//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef MAP_KEY_SET_REQUEST
#define MAP_KEY_SET_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class KeySetRequest : public Portable{
            public:
                KeySetRequest(const std::string& name)
                :name(name) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::KEY_SET;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer.writeUTF("name", name);
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    name = reader.readUTF("name");
                };
            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_KEY_SET_REQUEST
