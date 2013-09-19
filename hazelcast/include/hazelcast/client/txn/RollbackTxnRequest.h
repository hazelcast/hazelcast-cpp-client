//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RollbackTransactionRequest
#define HAZELCAST_RollbackTransactionRequest


#include "TxnPortableHook.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class RollbackTxnRequest : public Portable {
            public:
                RollbackTxnRequest() {

                };

                int getFactoryId() const {
                    return TxnPortableHook::F_ID;
                }

                int getClassId() const {
                    return TxnPortableHook::ROLLBACK;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                };


                void readPortable(serialization::PortableReader& reader) {
                };
            };
        }
    }
}


#endif //HAZELCAST_RollbackTransactionRequest
