//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/txn/CreateTxnRequest.h"
#include "hazelcast/client/txn/TxnPortableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            CreateTxnRequest::CreateTxnRequest(TransactionOptions options):options(options) {

            }

            int CreateTxnRequest::getFactoryId() const {
                return TxnPortableHook::F_ID;
            }

            int CreateTxnRequest::getClassId() const {
                return TxnPortableHook::CREATE;
            }

            void CreateTxnRequest::write(serialization::PortableWriter &writer) const {
                BaseTxnRequest::write(writer);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                options.writeData(out);
                out.writeBoolean(false);//SerializableXID null
            }

        }
    }
}

