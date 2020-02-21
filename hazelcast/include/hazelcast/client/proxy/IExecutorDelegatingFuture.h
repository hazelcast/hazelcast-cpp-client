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
#ifndef HAZELCAST_CLIENT_PROXY_IEXECUTORDELEGATINGPROXY_H_
#define HAZELCAST_CLIENT_PROXY_IEXECUTORDELEGATINGPROXY_H_

#include "hazelcast/client/internal/ClientDelegatingFuture.h"
#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/protocol/codec/ExecutorServiceCancelOnPartitionCodec.h"
#include "hazelcast/client/protocol/codec/ExecutorServiceCancelOnAddressCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            /**
             * An Abstract DelegatingFuture that can cancel a Runnable/Callable that is executed by an
             * {@link IExecutorService}.
             * It does this by sending a Cancellation Request to the remote owning member and then cancelling the running task.
             *
             * @param <V> Type of returned object from the get method of this class.
             */
            template<typename V>
            class IExecutorDelegatingFuture : public internal::ClientDelegatingFuture<V> {
            public:
                IExecutorDelegatingFuture(
                        const std::shared_ptr<spi::impl::ClientInvocationFuture> &clientInvocationFuture,
                        spi::ClientContext &context, const std::string &uuid, const std::shared_ptr<V> &defaultValue,
                        const std::shared_ptr<impl::ClientMessageDecoder<V> > &clientMessageDecoder,
                        const std::string &objectName, const int partitionId)
                        : internal::ClientDelegatingFuture<V>(clientInvocationFuture,
                                                                    context.getSerializationService(),
                                                                    clientMessageDecoder, defaultValue),
                          context(context), uuid(uuid), partitionId(partitionId), objectName(objectName) {}

                IExecutorDelegatingFuture(
                        const std::shared_ptr<spi::impl::ClientInvocationFuture> &clientInvocationFuture,
                        spi::ClientContext &context, const std::string &uuid, const std::shared_ptr<V> &defaultValue,
                        const std::shared_ptr<impl::ClientMessageDecoder<V> > &clientMessageDecoder,
                        const std::string &objectName, const Address &address)
                        : internal::ClientDelegatingFuture<V>(clientInvocationFuture,
                                                                    context.getSerializationService(),
                                                                    clientMessageDecoder, defaultValue),
                          context(context), uuid(uuid), target(new Address(address)), partitionId(-1), objectName(objectName) {}

                virtual bool cancel(bool mayInterruptIfRunning) {
                    if (internal::ClientDelegatingFuture<V>::isDone()) {
                        return false;
                    }

                    bool cancelSuccessful = false;
                    try {
                        cancelSuccessful = invokeCancelRequest(mayInterruptIfRunning);
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }

                    internal::ClientDelegatingFuture<V>::complete(std::shared_ptr<exception::IException>(
                            new exception::CancellationException("IExecutorDelegatingFuture::cancel(bool)")));
                    return cancelSuccessful;
                }

            private:
                void waitForRequestToBeSend() {
                    std::shared_ptr<spi::impl::ClientInvocationFuture> future = internal::ClientDelegatingFuture<V>::getFuture();
                    auto invocation = future->getInvocation();
                    if (!invocation.get()) {
                        return;
                    }

                    invocation->getSendConnectionOrWait();
                }

                bool invokeCancelRequest(bool mayInterruptIfRunning) {
                    waitForRequestToBeSend();

                    if (partitionId > -1) {
                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceCancelOnPartitionCodec::encodeRequest(
                                uuid, partitionId,
                                mayInterruptIfRunning);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context, request, objectName, partitionId);
                        std::shared_ptr<spi::impl::ClientInvocationFuture> f = clientInvocation->invoke();
                        return protocol::codec::ExecutorServiceCancelOnPartitionCodec::ResponseParameters::decode(
                                *f->get()).response;
                    } else {
                        std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ExecutorServiceCancelOnAddressCodec::encodeRequest(
                                uuid, *target, mayInterruptIfRunning);
                        std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                                context, request, objectName, *target);
                        std::shared_ptr<spi::impl::ClientInvocationFuture> f = clientInvocation->invoke();
                        return protocol::codec::ExecutorServiceCancelOnAddressCodec::ResponseParameters::decode(
                                *f->get()).response;
                    }
                }

                spi::ClientContext &context;
                const std::string uuid;
                const std::shared_ptr<Address> target;
                const int partitionId;
                const std::string objectName;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_IEXECUTORDELEGATINGPROXY_H_
