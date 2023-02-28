/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
class data;
}
} // namespace serialization
namespace proxy {
class HAZELCAST_API TransactionalMultiMapImpl : public TransactionalObject
{
public:
    /**
     * Transactional implementation of Multimap#size().
     *
     * @see Multimap#size()
     */
    boost::future<int> size();

protected:
    TransactionalMultiMapImpl(const std::string& name,
                              txn::TransactionProxy& transaction_proxy);

    boost::future<bool> put_data(const serialization::pimpl::data& key,
                                 const serialization::pimpl::data& value);

    boost::future<std::vector<serialization::pimpl::data>> get_data(
      const serialization::pimpl::data& key);

    boost::future<bool> remove(const serialization::pimpl::data& key,
                               const serialization::pimpl::data& value);

    boost::future<std::vector<serialization::pimpl::data>> remove_data(
      const serialization::pimpl::data& key);

    boost::future<int> value_count(const serialization::pimpl::data& key);
};
} // namespace proxy
} // namespace client
} // namespace hazelcast
