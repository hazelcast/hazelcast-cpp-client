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
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace proxy {
            class HAZELCAST_API TransactionalMultiMapImpl : public TransactionalObject {
            public:
                /**
                * Transactional implementation of Multimap#size().
                *
                * @see Multimap#size()
                */
                boost::future<int> size();

            protected:
                TransactionalMultiMapImpl(const std::string &name, txn::TransactionProxy &transactionProxy);

                boost::future<bool>
                putData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<std::vector<serialization::pimpl::Data>> getData(const serialization::pimpl::Data &key);

                boost::future<bool>
                remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<std::vector<serialization::pimpl::Data>>
                removeData(const serialization::pimpl::Data &key);

                boost::future<int> valueCount(const serialization::pimpl::Data &key);
            };
        }
    }
}
