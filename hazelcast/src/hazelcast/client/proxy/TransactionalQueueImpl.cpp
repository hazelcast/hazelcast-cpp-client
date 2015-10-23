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

#include "hazelcast/client/proxy/TransactionalQueueImpl.h"
#include "hazelcast/client/queue/TxnOfferRequest.h"
#include "hazelcast/client/queue/TxnPollRequest.h"
#include "hazelcast/client/queue/TxnSizeRequest.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalQueueImpl::TransactionalQueueImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }

            bool TransactionalQueueImpl::offer(const serialization::pimpl::Data& e, long timeoutInMillis) {
                queue::TxnOfferRequest *request = new queue::TxnOfferRequest(getName(), timeoutInMillis, e);
                boost::shared_ptr<bool> result = toObject<bool>(invoke(request));
                return *result;
            }

            serialization::pimpl::Data TransactionalQueueImpl::poll(long timeoutInMillis) {
                queue::TxnPollRequest *request = new queue::TxnPollRequest(getName(), timeoutInMillis);
                return invoke(request);
            }

            int TransactionalQueueImpl::size() {
                queue::TxnSizeRequest *request = new queue::TxnSizeRequest(getName());
                boost::shared_ptr<int> result = toObject<int>(invoke(request));
                return *result;
            }
        }
    }
}

