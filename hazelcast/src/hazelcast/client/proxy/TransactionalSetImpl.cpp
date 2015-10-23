/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/TransactionalSetImpl.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/TxnSetAddRequest.h"
#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/collection/TxnSetRemoveRequest.h"
#include "hazelcast/client/collection/TxnSetSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalSetImpl::TransactionalSetImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:setService", name, transactionProxy) {

            }

            bool TransactionalSetImpl::add(const serialization::pimpl::Data& e) {
                collection::TxnSetAddRequest *request = new collection::TxnSetAddRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            bool TransactionalSetImpl::remove(const serialization::pimpl::Data& e) {
                collection::TxnSetRemoveRequest *request = new collection::TxnSetRemoveRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            int TransactionalSetImpl::size() {
                collection::TxnSetSizeRequest *request = new collection::TxnSetSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }
        }
    }
}

