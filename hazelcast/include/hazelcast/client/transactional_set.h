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

#include "hazelcast/client/proxy/TransactionalSetImpl.h"

namespace hazelcast {
namespace client {
/**
 * Transactional implementation of iset.
 */
class transactional_set : public proxy::TransactionalSetImpl
{
    friend class transaction_context;

public:
    /**
     * Add new item to transactional set
     * @param e item
     * @return true if item is added successfully
     */
    template<typename E>
    boost::future<bool> add(const E& e)
    {
        return proxy::TransactionalSetImpl::add_data(to_data(e));
    }

    /**
     * Add item from transactional set
     * @param e item
     * @return true if item is remove successfully
     */
    template<typename E>
    boost::future<bool> remove(const E& e)
    {
        return proxy::TransactionalSetImpl::remove_data(to_data(e));
    }

private:
    transactional_set(const std::string& name,
                      txn::TransactionProxy& transaction_proxy)
      : TransactionalSetImpl(name, transaction_proxy)
    {}
};
} // namespace client
} // namespace hazelcast
