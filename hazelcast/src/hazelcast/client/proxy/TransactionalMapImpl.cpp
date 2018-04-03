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
#include "hazelcast/client/proxy/TransactionalMapImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/TransactionalMapContainsKeyCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapGetCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapSizeCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapIsEmptyCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapPutCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapSetCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapPutIfAbsentCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapReplaceCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapReplaceIfSameCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapRemoveCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapDeleteCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapRemoveIfSameCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapKeySetCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapKeySetWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapValuesCodec.h"
#include "hazelcast/client/protocol/codec/TransactionalMapValuesWithPredicateCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalMapImpl::TransactionalMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:mapService", name, transactionProxy) {

            }

            bool TransactionalMapImpl::containsKey(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapContainsKeyCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapContainsKeyCodec::ResponseParameters>(request);
            }

            std::auto_ptr<serialization::pimpl::Data> TransactionalMapImpl::getData(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapGetCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapGetCodec::ResponseParameters>(request);
            }

            int TransactionalMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapSizeCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<int, protocol::codec::TransactionalMapSizeCodec::ResponseParameters>(request);
            }

            std::auto_ptr<serialization::pimpl::Data> TransactionalMapImpl::putData(
                    const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapPutCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value, getTimeoutInMilliseconds());

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapPutCodec::ResponseParameters>(request);

            }

            void TransactionalMapImpl::set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapSetCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                invoke(request);
            }

            std::auto_ptr<serialization::pimpl::Data> TransactionalMapImpl::putIfAbsentData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapPutIfAbsentCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapPutIfAbsentCodec::ResponseParameters>(request);
            }

            std::auto_ptr<serialization::pimpl::Data> TransactionalMapImpl::replaceData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapReplaceCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapReplaceCodec::ResponseParameters>(request);
            }

            bool TransactionalMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapReplaceIfSameCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, oldValue, newValue);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapReplaceIfSameCodec::ResponseParameters>(request);
            }

            std::auto_ptr<serialization::pimpl::Data> TransactionalMapImpl::removeData(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapRemoveCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::TransactionalMapRemoveCodec::ResponseParameters>(request);
            }

            void TransactionalMapImpl::deleteEntry(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapDeleteCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                invoke(request);
            }

            bool TransactionalMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapRemoveIfSameCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<bool, protocol::codec::TransactionalMapRemoveIfSameCodec::ResponseParameters>(request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::keySetData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapKeySetCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapKeySetCodec::ResponseParameters>(request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::keySetData(const serialization::IdentifiedDataSerializable *predicate) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapKeySetWithPredicateCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapKeySetWithPredicateCodec::ResponseParameters>(request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::valuesData() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapValuesCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapValuesCodec::ResponseParameters>(request);
            }

            std::vector<serialization::pimpl::Data> TransactionalMapImpl::valuesData(const serialization::IdentifiedDataSerializable *predicate) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::TransactionalMapValuesWithPredicateCodec::RequestParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), toData<serialization::IdentifiedDataSerializable>(predicate));

                return invokeAndGetResult<std::vector<serialization::pimpl::Data>, protocol::codec::TransactionalMapValuesWithPredicateCodec::ResponseParameters>(request);
            }
        }
    }
}

