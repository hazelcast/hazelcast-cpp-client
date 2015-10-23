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

#include "hazelcast/client/proxy/TransactionalListImpl.h"
#include "hazelcast/client/collection/TxnListAddRequest.h"
#include "hazelcast/client/collection/TxnListRemoveRequest.h"
#include "hazelcast/client/collection/TxnListSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalListImpl::TransactionalListImpl(const std::string& objectName, txn::TransactionProxy *context)
            : TransactionalObject("hz:impl:listService", objectName, context) {
            }

            bool TransactionalListImpl::add(const serialization::pimpl::Data& e) {
                collection::TxnListAddRequest *request = new collection::TxnListAddRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            bool TransactionalListImpl::remove(const serialization::pimpl::Data& e) {
                collection::TxnListRemoveRequest *request = new collection::TxnListRemoveRequest(getName(), e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            int TransactionalListImpl::size() {
                collection::TxnListSizeRequest *request = new collection::TxnListSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }
        }
    }
}

