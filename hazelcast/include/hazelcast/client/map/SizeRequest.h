//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_SIZE_REQUEST
#define HAZELCAST_MAP_SIZE_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class SizeRequest : public Portable{
            public:
                SizeRequest(const std::string& name)
                :name(name) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::SIZE;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                };
            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST
