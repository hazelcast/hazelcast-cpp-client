//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapRemoveRequest
#define HAZELCAST_TxnMultiMapRemoveRequest

#include "TxnMultiMapRequest.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnMultiMapRemoveRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key);

                TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key, serialization::Data *value);

                int getClassId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    util::writeNullableData(out, value);
                };


                void readPortable(serialization::PortableReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                    value = new serialization::Data();
                    value->readData(in);
                };
            private:
                serialization::Data& key;
                serialization::Data *value;

            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapRemoveRequest
