//
// Created by sancar koyunlu on 8/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/TransactionalTaskContext.h"


namespace hazelcast {
    namespace client {
        TransactionalTaskContext::TransactionalTaskContext(TransactionContext& context)
        : context(context) {

        }
    }
}