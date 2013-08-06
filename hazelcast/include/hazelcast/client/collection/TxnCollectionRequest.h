//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnCollectionRequest
#define HAZELCAST_TxnCollectionRequest

#include "Data.h"
#include "Portable.h"
#include "CollectionPortableHook.h"
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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    util::writeNullableData(out, data);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    data->readData(*in);
                };

            private:
                std::string name;
                serialization::Data *data;
            };
        }
    }
}


#endif //HAZELCAST_TxnCollectionRequest
