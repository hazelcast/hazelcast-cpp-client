//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapGetRequest
#define HAZELCAST_TxnMultiMapGetRequest

#include "TxnMultiMapRequest.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnMultiMapGetRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapGetRequest(const std::string& name, serialization::Data& data);

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    data.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    data.readData(*in);
                };
            private:
                serialization::Data& data;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapGetRequest
