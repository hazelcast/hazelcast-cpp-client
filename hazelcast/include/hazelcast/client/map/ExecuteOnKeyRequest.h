//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ExecuteOnKeyRequest
#define HAZELCAST_ExecuteOnKeyRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            template<typename EntryProcessor>
            class HAZELCAST_API ExecuteOnKeyRequest : public impl::PortableRequest {
            public:
                ExecuteOnKeyRequest(const std::string name, EntryProcessor& entryProcessor, serialization::Data& key)
                :name(name)
                , entryProcessor(entryProcessor)
                , key(key) {

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::EXECUTE_ON_KEY;
                }

                void writePortable(serialization::PortableWriter & writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    out.writeObject(&entryProcessor);
                }

            private:
                std::string name;
                EntryProcessor& entryProcessor;
                serialization::Data& key;
            };
        }
    }
}

#endif //HAZELCAST_ExecuteOnKeyRequest

