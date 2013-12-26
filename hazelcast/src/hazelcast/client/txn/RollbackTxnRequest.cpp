//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/txn/RollbackTxnRequest.h"
#include "hazelcast/client/txn/TxnPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace txn {

            int RollbackTxnRequest::getFactoryId() const {
                return TxnPortableHook::F_ID;
            }

            int RollbackTxnRequest::getClassId() const {
                return TxnPortableHook::ROLLBACK;
            }

            void RollbackTxnRequest::write(serialization::PortableWriter &writer) const {
            };

        }
    }
}

