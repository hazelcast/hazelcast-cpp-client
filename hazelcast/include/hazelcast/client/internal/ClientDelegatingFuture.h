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
#ifndef HAZELCAST_CLIENT_INTERNAL_CLIENTDELEGATINGFUTURE_H
#define HAZELCAST_CLIENT_INTERNAL_CLIENTDELEGATINGFUTURE_H

#include <assert.h>

#include <hazelcast/client/spi/InternalCompletableFuture.h>
#include <hazelcast/client/spi/impl/ClientInvocationFuture.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>
#include <hazelcast/client/impl/ClientMessageDecoder.h>
#include <hazelcast/client/spi/impl/ClientInvocation.h>

namespace hazelcast {
    namespace client {
        namespace internal {
            /**
             * The Client Delegating Future is used to delegate {@link
             * ClientInvocationFuture} to a user type to be used with {@code andThen()} or
             * {@code get()}. It converts {@link ClientMessage} coming from {@link
             * ClientInvocationFuture} to a user object.
             *
             * @param <V> Value type that the user expects
             */
            template<typename V>
            class ClientDelegatingFuture
                    : public spi::InternalCompletableFuture<V>,
                      public std::enable_shared_from_this<ClientDelegatingFuture<V> > {
            public:
                ClientDelegatingFuture(
                        const std::shared_ptr<spi::impl::ClientInvocationFuture> &clientInvocationFuture,
                        serialization::pimpl::SerializationService &serializationService,
                        const std::shared_ptr<impl::ClientMessageDecoder<V> > &clientMessageDecoder,
                        const std::shared_ptr<V> &defaultValue) : future(
                        clientInvocationFuture), serializationService(serializationService), clientMessageDecoder(
                        clientMessageDecoder), defaultValue(defaultValue), decodedResponse(
                        std::static_pointer_cast<V>(VOIDOBJ)) {
                    const std::shared_ptr<spi::impl::ClientInvocation> invocation = clientInvocationFuture->getInvocation();
                    if (invocation.get()) {
                        userExecutor = invocation->getUserExecutor();
                    }
                }

                ClientDelegatingFuture(
                        const std::shared_ptr<spi::impl::ClientInvocationFuture> &clientInvocationFuture,
                        serialization::pimpl::SerializationService &serializationService,
                        const std::shared_ptr<impl::ClientMessageDecoder<V> > &clientMessageDecoder) : future(
                        clientInvocationFuture), serializationService(serializationService), clientMessageDecoder(
                        clientMessageDecoder), decodedResponse(std::static_pointer_cast<V>(VOIDOBJ)) {
                    const std::shared_ptr<spi::impl::ClientInvocation> invocation = clientInvocationFuture->getInvocation();
                    if (invocation.get()) {
                        userExecutor = invocation->getUserExecutor();
                    }
                }

                virtual void andThen(const std::shared_ptr<ExecutionCallback<V> > &callback) {
                    future->andThen(std::shared_ptr<ExecutionCallback<protocol::ClientMessage> >(
                            new DelegatingExecutionCallback(
                                    std::enable_shared_from_this<ClientDelegatingFuture<V> >::shared_from_this(),
                                    callback)), userExecutor);
                }

                virtual void andThen(const std::shared_ptr<ExecutionCallback<V> > &callback,
                                     const std::shared_ptr<Executor> &executor) {
                    future->andThen(std::shared_ptr<ExecutionCallback<protocol::ClientMessage> >(
                            new DelegatingExecutionCallback(
                                    std::enable_shared_from_this<ClientDelegatingFuture<V> >::shared_from_this(),
                                    callback)), executor);
                }

                virtual bool cancel(bool mayInterruptIfRunning) {
                    return future->cancel(mayInterruptIfRunning);
                }

                virtual bool isCancelled() {
                    return future->isCancelled();
                }

                virtual bool isDone() {
                    return future->isDone();
                }

                virtual std::shared_ptr<V> get() {
                    return get(INT64_MAX, hazelcast::util::concurrent::TimeUnit::MILLISECONDS());
                }

                virtual std::shared_ptr<V> get(int64_t timeout, const TimeUnit &unit) {
                    std::shared_ptr<protocol::ClientMessage> response = future->get(timeout, unit);
                    return resolveResponse(response);
                }

                virtual std::shared_ptr<V> join() {
                    try {
                        return get();
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return std::shared_ptr<V>();
                }

                virtual bool complete(const std::shared_ptr<V> &value) {
                    assert(0);
                    return false;
                }

                virtual bool complete(const std::shared_ptr<exception::IException> &value) {
                    return future->complete(value);
                }

            protected:
                const std::shared_ptr<spi::impl::ClientInvocationFuture> &getFuture() const {
                    return future;
                }

            private:
                class DelegatingExecutionCallback : public ExecutionCallback<protocol::ClientMessage> {
                public:
                    DelegatingExecutionCallback(
                            const std::shared_ptr<ClientDelegatingFuture<V> > &delegatingFuture,
                            const std::shared_ptr<ExecutionCallback<V> > &callback) : delegatingFuture(
                            delegatingFuture), callback(callback) {}

                    virtual void onResponse(const std::shared_ptr<protocol::ClientMessage> &message) {
                        std::shared_ptr<V> response = delegatingFuture->resolveResponse(message);
                        callback->onResponse(response);
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                        callback->onFailure(e);
                    }

                private:
                    std::shared_ptr<ClientDelegatingFuture> delegatingFuture;
                    const std::shared_ptr<ExecutionCallback<V> > callback;
                };

                std::shared_ptr<V>
                decodeResponse(const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    if (decodedResponse != std::static_pointer_cast<V>(VOIDOBJ)) {
                        return decodedResponse;
                    }

                    std::shared_ptr<protocol::ClientMessage> message(
                            protocol::ClientMessage::createForDecode(*clientMessage));
                    std::shared_ptr<V> newDecodedResponse = clientMessageDecoder->decodeClientMessage(message,
                                                                                                        serializationService);

                    decodedResponse.compareAndSet(std::static_pointer_cast<V>(VOIDOBJ), newDecodedResponse);
                    return newDecodedResponse;
                }

                std::shared_ptr<V>
                resolveResponse(const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    if (defaultValue.get() != NULL) {
                        return defaultValue;
                    }

                    return decodeResponse(clientMessage);
                }

                static const std::shared_ptr<void> VOIDOBJ;
                const std::shared_ptr<spi::impl::ClientInvocationFuture> future;
                serialization::pimpl::SerializationService &serializationService;
                const std::shared_ptr<impl::ClientMessageDecoder<V> > clientMessageDecoder;
                const std::shared_ptr<V> defaultValue;
                std::shared_ptr<hazelcast::util::Executor> userExecutor;
                util::Sync<std::shared_ptr<V> > decodedResponse;
            };

            template<typename V>
            const std::shared_ptr<void> ClientDelegatingFuture<V>::VOIDOBJ(new char);
        }
    }
};

#endif /* HAZELCAST_CLIENT_INTERNAL_CLIENTDELEGATINGFUTURE_H */
