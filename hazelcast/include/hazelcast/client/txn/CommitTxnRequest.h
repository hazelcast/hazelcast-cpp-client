//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CommitTxnRequest
#define HAZELCAST_CommitTxnRequest

#include "TxnPortableHook.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class CommitTxnRequest : public Portable{
            public:
                CommitTxnRequest() {

                };

                int getFactoryId() const {
                    return TxnPortableHook::F_ID;
                }

                int getClassId() const {
                    return TxnPortableHook::COMMIT;
                }


                void writePortable(serialization::PortableWriter& writer) const {
                };


                void readPortable(serialization::PortableReader& reader) {
                };
            };
        }
    }
}

#endif //HAZELCAST_CommitTxnRequest
