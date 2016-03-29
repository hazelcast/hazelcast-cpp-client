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

#include "hazelcast/util/Util.h"
#include "hazelcast/client/proxy/TransactionalQueueImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/TransactionalQueueOfferCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalQueueTakeCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalQueuePollCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalQueuePeekCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalQueueSizeCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalQueueImpl::TransactionalQueueImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }

            bool TransactionalQueueImpl::offer(const serialization::pimpl::Data& e, long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalQueueOfferCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), e, timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::TransactionalQueueOfferCodec::ResponseParameters>(request);
            }

            std::auto_ptr<serialization::pimpl::Data> TransactionalQueueImpl::pollData(long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalQueuePollCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), timeoutInMillis);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalQueuePollCodec::ResponseParameters>(request);
            }

            int TransactionalQueueImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalQueueSizeCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalQueueSizeCodec::ResponseParameters>(request);
            }
        }
    }
}

