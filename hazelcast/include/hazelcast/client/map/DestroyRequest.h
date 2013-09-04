//
// Created by sancar koyunlu on 6/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_MAP_DESTROY_REQUEST
#define HAZELCAST_MAP_DESTROY_REQUEST

#include "PortableHook.h"

#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class DestroyRequest : public Portable, public RetryableRequest {
            public:
                DestroyRequest(const std::string& name)
                :name(name) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::DESTROY;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("name", name);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("name");
                };
            private:
                std::string name;
            };
        }
    }
}


#endif //MAP_CLEAR_REQUEST
