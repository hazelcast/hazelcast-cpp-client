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
#include "hazelcast/client/proxy/ClientAtomicLongProxy.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientAtomicLongProxy::SERVICE_NAME = "hz:impl:atomicLongService";

            ClientAtomicLongProxy::ClientAtomicLongProxy(const std::string &objectName, spi::ClientContext *context)
                    : proxy::ProxyImpl(SERVICE_NAME, objectName, context) {
                serialization::pimpl::Data keyData = toData<std::string>(objectName);
                partitionId = getPartitionId(keyData);
            }

            int64_t ClientAtomicLongProxy::addAndGet(int64_t delta) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongAddAndGetCodec::encodeRequest(getName(), delta);

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongAddAndGetCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool ClientAtomicLongProxy::compareAndSet(int64_t expect, int64_t update) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongCompareAndSetCodec::encodeRequest(getName(), expect, update);

                return invokeAndGetResult<bool, protocol::codec::AtomicLongCompareAndSetCodec::ResponseParameters>(
                        request, partitionId);
            }

            int64_t ClientAtomicLongProxy::decrementAndGet() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongDecrementAndGetCodec::encodeRequest(getName());

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongDecrementAndGetCodec::ResponseParameters>(
                        request, partitionId);
            }

            int64_t ClientAtomicLongProxy::get() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetCodec::encodeRequest(getName());

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetCodec::ResponseParameters>(request,
                                                                                                            partitionId);
            }

            int64_t ClientAtomicLongProxy::getAndAdd(int64_t delta) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndAddCodec::encodeRequest(getName(), delta);

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndAddCodec::ResponseParameters>(
                        request, partitionId);
            }

            int64_t ClientAtomicLongProxy::getAndSet(int64_t newValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndSetCodec::encodeRequest(getName(), newValue);

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndSetCodec::ResponseParameters>(
                        request, partitionId);
            }

            int64_t ClientAtomicLongProxy::incrementAndGet() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongIncrementAndGetCodec::encodeRequest(getName());

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongIncrementAndGetCodec::ResponseParameters>(
                        request, partitionId);
            }

            int64_t ClientAtomicLongProxy::getAndIncrement() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndIncrementCodec::encodeRequest(getName());

                return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndIncrementCodec::ResponseParameters>(
                        request, partitionId);
            }

            void ClientAtomicLongProxy::set(int64_t newValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongSetCodec::encodeRequest(getName(), newValue);

                invokeOnPartition(request, partitionId);
            }
        }
    }
}
