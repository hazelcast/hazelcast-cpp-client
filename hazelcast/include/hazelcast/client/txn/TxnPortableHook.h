//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnPortableHook
#define HAZELCAST_TxnPortableHook

namespace hazelcast {
    namespace client {
        namespace txn {
            namespace TxnPortableHook {
                enum {
                    F_ID = -19,
                    CREATE = 1,
                    COMMIT = 2,
                    ROLLBACK = 3,
                };
            }

        }
    }
}


#endif //HAZELCAST_TxnPortableHook
