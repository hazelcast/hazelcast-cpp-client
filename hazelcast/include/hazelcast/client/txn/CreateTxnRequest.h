//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CreateTxnRequest
#define HAZELCAST_CreateTxnRequest

#include "TxnPortableHook.h"
#include "Portable.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "TransactionOptions.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class CreateTxnRequest : public Portable {
            public:
                CreateTxnRequest() {

                };

                CreateTxnRequest(TransactionOptions options):options(options) {

                };

                int getFactoryId() const {
                    return TxnPortableHook::F_ID;
                }

                int getClassId() const {
                    return TxnPortableHook::CREATE;
                }

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    options.writeData(*out);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    options.readData(*in);
                };

            private:
                TransactionOptions options;
            };
        }
    }
}


#endif //HAZELCAST_CreateTxnRequest
