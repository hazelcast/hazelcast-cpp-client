/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/proxy/TransactionalSetImpl.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/txn/TransactionProxy.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/TransactionalSetAddCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalSetRemoveCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalSetSizeCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalSetImpl::TransactionalSetImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:setService", name, transactionProxy) {

            }

            bool TransactionalSetImpl::add(const serialization::pimpl::Data& e) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalSetAddCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetResult<bool, protocol::codec::TransactionalSetAddCodec::ResponseParameters>(request);
            }

            bool TransactionalSetImpl::remove(const serialization::pimpl::Data& e) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalSetRemoveCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e);

                return invokeAndGetResult<bool, protocol::codec::TransactionalSetRemoveCodec::ResponseParameters>(request);
            }

            int TransactionalSetImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalSetSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalSetSizeCodec::ResponseParameters>(request);
            }
        }
    }
}

