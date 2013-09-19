//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnOfferRequest
#define HAZELCAST_TxnOfferRequest

#include "Portable.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class TxnOfferRequest : public Portable {
            public:
                TxnOfferRequest(const std::string& name, long timeoutInMillis, serialization::Data&);

                int getFactoryId() const;

                int getClassId() const;


                void writePortable(serialization::PortableWriter& writer) const {
                    writer.writeUTF("n", name);
                    writer.writeLong("t", timeoutInMillis);
                    serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                    data.writeData(out);
                };


                void readPortable(serialization::PortableReader& reader) {
                    name = reader.readUTF("n");
                    timeoutInMillis = reader.readLong("t");
                    serialization::ObjectDataInput &in = reader.getRawDataInput();
                    data.readData(in);
                };
            private:
                std::string name;
                long timeoutInMillis;
                serialization::Data& data;
            };
        }
    }
}

#endif //HAZELCAST_TxnOfferRequest
