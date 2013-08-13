//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnCollectionRequest
#define HAZELCAST_TxnCollectionRequest

#include "Data.h"
#include "Portable.h"
#include "CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnCollectionRequest : public Portable {
            public:
                TxnCollectionRequest(const std::string& name);

                TxnCollectionRequest(const std::string& name, serialization::Data *);

                int getFactoryId() const {
                    return CollectionPortableHook::F_ID;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    util::writeNullableData(out, data);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    data->readData(in);
                };

            private:
                std::string name;
                serialization::Data *data;
            };
        }
    }
}


#endif //HAZELCAST_TxnCollectionRequest
