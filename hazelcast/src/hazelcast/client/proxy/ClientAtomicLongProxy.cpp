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
#include "hazelcast/client/proxy/ClientAtomicLongProxy.h"
#include "hazelcast/client/spi/InternalCompletableFuture.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string ClientAtomicLongProxy::SERVICE_NAME = "hz:impl:atomicLongService";

            ClientAtomicLongProxy::ClientAtomicLongProxy(const std::string &objectName, spi::ClientContext *context)
                    : PartitionSpecificClientProxy(SERVICE_NAME, objectName, context) {
            }

            int64_t ClientAtomicLongProxy::addAndGet(int64_t delta) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        addAndGetAsync(delta))->join());
            }

            bool ClientAtomicLongProxy::compareAndSet(int64_t expect, int64_t update) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<bool> >(
                        compareAndSetAsync(expect, update))->join());
            }

            int64_t ClientAtomicLongProxy::decrementAndGet() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        decrementAndGetAsync())->join());
            }

            int64_t ClientAtomicLongProxy::get() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(getAsync())->join());
            }

            int64_t ClientAtomicLongProxy::getAndAdd(int64_t delta) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        getAndAddAsync(delta))->join());
            }

            int64_t ClientAtomicLongProxy::getAndSet(int64_t newValue) {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        getAndSetAsync(newValue))->join());
            }

            int64_t ClientAtomicLongProxy::incrementAndGet() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        incrementAndGetAsync())->join());
            }

            int64_t ClientAtomicLongProxy::getAndIncrement() {
                return *(std::static_pointer_cast<spi::InternalCompletableFuture<int64_t> >(
                        getAndIncrementAsync())->join());
            }

            void ClientAtomicLongProxy::set(int64_t newValue) {
                std::static_pointer_cast<spi::InternalCompletableFuture<void> >(setAsync(newValue))->join();
            }

            std::shared_ptr<ICompletableFuture<int64_t> >
            ClientAtomicLongProxy::addAndGetAsync(int64_t delta) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongAddAndGetCodec::encodeRequest(name, delta);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongAddAndGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<bool> >
            ClientAtomicLongProxy::compareAndSetAsync(int64_t expect, int64_t update) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongCompareAndSetCodec::encodeRequest(name, expect, update);

                return invokeOnPartitionAsync<bool>(request,
                                                    impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongCompareAndSetCodec, bool>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::decrementAndGetAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongDecrementAndGetCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongDecrementAndGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::getAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> >
            ClientAtomicLongProxy::getAndAddAsync(int64_t delta) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndAddCodec::encodeRequest(name, delta);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetAndAddCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> >
            ClientAtomicLongProxy::getAndSetAsync(int64_t newValue) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndSetCodec::encodeRequest(name, newValue);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetAndSetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::incrementAndGetAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongIncrementAndGetCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongIncrementAndGetCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<int64_t> > ClientAtomicLongProxy::getAndIncrementAsync() {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongGetAndIncrementCodec::encodeRequest(name);

                return invokeOnPartitionAsync<int64_t>(request,
                                                       impl::PrimitiveMessageDecoder<protocol::codec::AtomicLongGetAndIncrementCodec, int64_t>::instance());
            }

            std::shared_ptr<ICompletableFuture<void> > ClientAtomicLongProxy::setAsync(int64_t newValue) {
                std::unique_ptr<protocol::ClientMessage> request =
                        protocol::codec::AtomicLongSetCodec::encodeRequest(name, newValue);

                return invokeOnPartitionAsync<void>(request, impl::VoidMessageDecoder::instance());
            }
        }
    }
}
