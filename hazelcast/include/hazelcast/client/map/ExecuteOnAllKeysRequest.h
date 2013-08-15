//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ExecuteOnEntriesRequest
#define HAZELCAST_ExecuteOnEntriesRequest

#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "PortableHook.h"
#include "Data.h"
#include <string>

namespace hazelcast {
    namespace client {

        namespace map {
            template<typename EntryProcessor>
            class ExecuteOnAllKeysRequest : public Portable {
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

                void writePortable(serialization::PortableWriter & writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    out.writeObject<EntryProcessor>(&entryProcessor);
                }

                void readPortable(serialization::PortableReader & reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput& in = reader.getRawDataInput();
                    entryProcessor = in.readObject<EntryProcessor>();
                }

            private:
                std::string name;
                EntryProcessor& entryProcessor;
            };
        }
    }
}


#endif //HAZELCAST_ExecuteOnEntriesRequest
