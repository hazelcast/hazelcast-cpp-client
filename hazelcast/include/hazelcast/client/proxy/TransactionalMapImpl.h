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


#ifndef HAZELCAST_TransactionalMapProxy
#define HAZELCAST_TransactionalMapProxy

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            class HAZELCAST_API TransactionalMapImpl : public TransactionalObject {
            protected:

                bool containsKey(const serialization::pimpl::Data& key);

                std::auto_ptr<serialization::pimpl::Data> getData(const serialization::pimpl::Data& key);

                int size();

                std::auto_ptr<serialization::pimpl::Data> putData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::auto_ptr<serialization::pimpl::Data> putIfAbsentData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::auto_ptr<serialization::pimpl::Data> replaceData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                bool replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue);

                std::auto_ptr<serialization::pimpl::Data> removeData(const serialization::pimpl::Data& key);

                void deleteEntry(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::vector<serialization::pimpl::Data> keySetData();

                std::vector<serialization::pimpl::Data> keySetData(const serialization::IdentifiedDataSerializable *predicate);

                std::vector<serialization::pimpl::Data> valuesData();

                std::vector<serialization::pimpl::Data> valuesData(const serialization::IdentifiedDataSerializable *predicate);

                TransactionalMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy);
            };
        }
    }
}
#endif //HAZELCAST_TransactionalMapProxy
