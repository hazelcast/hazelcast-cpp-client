//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ExecuteOnEntriesRequest
#define HAZELCAST_ExecuteOnEntriesRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {

        namespace map {
            template<typename EntryProcessor>
            class HAZELCAST_API ExecuteOnAllKeysRequest : public impl::PortableRequest {
            public:
                ExecuteOnAllKeysRequest(const std::string name, EntryProcessor& entryProcessor)
                :name(name)
                , entryProcessor(entryProcessor) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::EXECUTE_ON_ALL_KEYS;
                }

                void write(serialization::PortableWriter & writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    out.writeObject<EntryProcessor>(&entryProcessor);
                }

            private:
                std::string name;
                EntryProcessor& entryProcessor;
            };
        }
    }
}


#endif //HAZELCAST_ExecuteOnEntriesRequest
