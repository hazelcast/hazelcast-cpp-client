//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapPutRequest
#define HAZELCAST_TxnMultiMapPutRequest

#include "TxnMultiMapRequest.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnMultiMapPutRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapPutRequest(const std::string& name, serialization::Data& key, serialization::Data& value);

                int getClassId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    key.writeData(out);
                    value.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    key.readData(in);
                    value.readData(in);
                };
            private:
                serialization::Data& key;
                serialization::Data& value;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapPutRequest
