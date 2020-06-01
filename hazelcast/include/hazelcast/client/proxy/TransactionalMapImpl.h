/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/client/proxy/TransactionalObject.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            class HAZELCAST_API TransactionalMapImpl : public TransactionalObject {
            public:
                /**
                * Transactional implementation of IMap#size().
                *
                * @see IMap#size()
                */
                boost::future<int> size();

                /**
                * Transactional implementation of IMap#isEmpty().
                *
                * @see IMap#isEmpty()
                */
                boost::future<bool> isEmpty();

            protected:
                boost::future<bool> containsKeyData(const serialization::pimpl::Data &key);

                boost::future<std::unique_ptr<serialization::pimpl::Data>> getData(const serialization::pimpl::Data &key);

                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                putData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<void>
                setData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                putIfAbsentData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<std::unique_ptr<serialization::pimpl::Data>>
                replaceData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<bool>
                replaceData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                            const serialization::pimpl::Data &newValue);

                boost::future<std::unique_ptr<serialization::pimpl::Data>> removeData(const serialization::pimpl::Data &key);

                boost::future<void> deleteEntryData(const serialization::pimpl::Data &key);

                boost::future<bool>
                removeData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<std::vector<serialization::pimpl::Data>> keySetData();

                boost::future<std::vector<serialization::pimpl::Data>>
                keySetData(const serialization::pimpl::Data &predicate);

                boost::future<std::vector<serialization::pimpl::Data>> valuesData();

                boost::future<std::vector<serialization::pimpl::Data>>
                valuesData(const serialization::pimpl::Data &predicate);

                TransactionalMapImpl(const std::string &name, txn::TransactionProxy &transactionProxy);
            };
        }
    }
}
