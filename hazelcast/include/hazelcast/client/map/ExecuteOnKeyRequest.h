//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ExecuteOnKeyRequest
#define HAZELCAST_ExecuteOnKeyRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            template<typename EntryProcessor>
            class HAZELCAST_API ExecuteOnKeyRequest : public impl::ClientRequest {
            public:
                ExecuteOnKeyRequest(const std::string name, EntryProcessor &entryProcessor, serialization::pimpl::Data &key)
                :name(name)
                , entryProcessor(entryProcessor)
                , key(key)
                , submitToKey(false){

                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::EXECUTE_ON_KEY;
                }

                void write(serialization::PortableWriter &writer) const {
                    writer.writeUTF("n", name);
                    writer.writeBoolean("s", submitToKey);
                    serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                    key.writeData(out);
                    out.writeObject(&entryProcessor);
                }

            private:
                std::string name;
                EntryProcessor entryProcessor;
                bool submitToKey; //MTODO implement submitToKey request on IMAP
                serialization::pimpl::Data key;
            };
        }
    }
}

#endif //HAZELCAST_ExecuteOnKeyRequest


