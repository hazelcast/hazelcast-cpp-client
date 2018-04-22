/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/TransactionalMultiMapPutCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMultiMapGetCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMultiMapRemoveCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMultiMapRemoveEntryCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMultiMapValueCountCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMultiMapSizeCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            TransactionalMultiMapImpl::TransactionalMultiMapImpl(const std::string &name,
                                                                 txn::TransactionProxy *transactionProxy)
                    : TransactionalObject("hz:impl:multiMapService", name, transactionProxy) {

            }

            bool TransactionalMultiMapImpl::put(const serialization::pimpl::Data &key,
                                                const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapPutCodec::encodeRequest(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMultiMapPutCodec::ResponseParameters>(
                        request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMultiMapImpl::getData(
                    const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapGetCodec::encodeRequest(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMultiMapGetCodec::ResponseParameters>(
                        request);

            }

            bool TransactionalMultiMapImpl::remove(const serialization::pimpl::Data &key,
                                                   const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapRemoveEntryCodec::encodeRequest(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMultiMapRemoveEntryCodec::ResponseParameters>(
                        request);

            }

            std::vector<serialization::pimpl::Data> TransactionalMultiMapImpl::removeData(
                    const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapRemoveCodec::encodeRequest(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMultiMapRemoveCodec::ResponseParameters>(
                        request);

            }

            int TransactionalMultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapValueCountCodec::encodeRequest(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<int, protocol::codec::TransactionalMultiMapValueCountCodec::ResponseParameters>(
                        request);
            }

            int TransactionalMultiMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMultiMapSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalMultiMapSizeCodec::ResponseParameters>(
                        request);
            }

        }
    }
}
