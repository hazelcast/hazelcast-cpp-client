//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_ENTRY_SET_REQUEST
#define HAZELCAST_MAP_ENTRY_SET_REQUEST

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class EntrySetRequest : public Portable, public RetryableRequest {
            public:
                EntrySetRequest(const std::string& name)
                :name(name) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::ENTRY_SET;
                }


                inline void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("name", name);
                };


                inline void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("name");
                };
            private:
                std::string name;
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_REQUEST
