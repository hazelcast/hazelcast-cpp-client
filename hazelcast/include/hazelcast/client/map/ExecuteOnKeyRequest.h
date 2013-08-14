//
// Created by sancar koyunlu on 8/13/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ExecuteOnKeyRequest
#define HAZELCAST_ExecuteOnKeyRequest

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
            class ExecuteOnKeyRequest : public Portable {
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

                void readPortable(serialization::PortableReader & reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput& in = reader.getRawDataInput();
                    key.readData(in);
                    entryProcessor = in.readObject<EntryProcessor>();
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

