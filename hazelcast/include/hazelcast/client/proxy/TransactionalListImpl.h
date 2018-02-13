/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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


#ifndef HAZELCAST_TransactionalListProxy
#define HAZELCAST_TransactionalListProxy

#include "hazelcast/client/proxy/TransactionalObject.h"


namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API TransactionalListImpl : public proxy::TransactionalObject {
            public:
                TransactionalListImpl(const std::string& objectName, txn::TransactionProxy *context);

                bool add(const serialization::pimpl::Data& e);

                bool remove(const serialization::pimpl::Data& e);

                int size();

            };
        }
    }
}

#endif //HAZELCAST_TransactionalListProxy
