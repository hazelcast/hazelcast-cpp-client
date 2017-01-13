/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 01/10/14.
//
#ifndef HAZELCAST_TransactionalQueueProxy
#define HAZELCAST_TransactionalQueueProxy

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API TransactionalQueueImpl : public TransactionalObject {
            public:
                TransactionalQueueImpl(const std::string& name, txn::TransactionProxy *transactionProxy);

                bool offer(const serialization::pimpl::Data& e, long timeoutInMillis);

                std::auto_ptr<serialization::pimpl::Data> pollData(long timeoutInMillis);

                int size();
            };
        }
    }
}

#endif //HAZELCAST_TransactionalQueueProxy
