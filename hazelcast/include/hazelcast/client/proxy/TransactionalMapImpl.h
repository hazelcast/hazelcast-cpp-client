/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

class HAZELCAST_API TransactionalMapImpl : public TransactionalObject
{
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
    boost::future<bool> is_empty();

protected:
    boost::future<bool> contains_key_data(
      const serialization::pimpl::data& key);

    boost::future<boost::optional<serialization::pimpl::data>> get_data(
      const serialization::pimpl::data& key);

    boost::future<boost::optional<serialization::pimpl::data>> put_data(
      const serialization::pimpl::data& key,
      const serialization::pimpl::data& value);

    boost::future<void> set_data(const serialization::pimpl::data& key,
                                 const serialization::pimpl::data& value);

    boost::future<boost::optional<serialization::pimpl::data>>
    put_if_absent_data(const serialization::pimpl::data& key,
                       const serialization::pimpl::data& value);

    boost::future<boost::optional<serialization::pimpl::data>> replace_data(
      const serialization::pimpl::data& key,
      const serialization::pimpl::data& value);

    boost::future<bool> replace_data(
      const serialization::pimpl::data& key,
      const serialization::pimpl::data& old_value,
      const serialization::pimpl::data& new_value);

    boost::future<boost::optional<serialization::pimpl::data>> remove_data(
      const serialization::pimpl::data& key);

    boost::future<void> delete_entry_data(
      const serialization::pimpl::data& key);

    boost::future<bool> remove_data(const serialization::pimpl::data& key,
                                    const serialization::pimpl::data& value);

    boost::future<std::vector<serialization::pimpl::data>> key_set_data();

    boost::future<std::vector<serialization::pimpl::data>> key_set_data(
      const serialization::pimpl::data& predicate);

    boost::future<std::vector<serialization::pimpl::data>> values_data();

    boost::future<std::vector<serialization::pimpl::data>> values_data(
      const serialization::pimpl::data& predicate);

    TransactionalMapImpl(const std::string& name,
                         txn::TransactionProxy& transaction_proxy);
};
} // namespace proxy
} // namespace client
} // namespace hazelcast
