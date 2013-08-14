//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapValueCountRequest
#define HAZELCAST_TxnMultiMapValueCountRequest

#include "TxnMultiMapRequest.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnMultiMapValueCountRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapValueCountRequest(const std::string& name, serialization::Data& data);

                int getClassId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    TxnMultiMapRequest::writePortable(writer);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    data.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    TxnMultiMapRequest::readPortable(reader);
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    data.readData(in);
                };
            private:
                serialization::Data& data;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapValueCountRequest
