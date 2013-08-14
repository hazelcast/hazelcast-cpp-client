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

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    key.writeData(*out);
                    value.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    key.readData(*in);
                    value.readData(*in);
                };
            private:
                serialization::Data& key;
                serialization::Data& value;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapPutRequest
