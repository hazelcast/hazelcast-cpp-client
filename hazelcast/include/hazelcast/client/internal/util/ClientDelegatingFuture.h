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
#ifndef HAZELCAST_CLIENT_INTERNAL_UTIL_CLIENTDELEGATINGFUTURE_H_
#define HAZELCAST_CLIENT_INTERNAL_UTIL_CLIENTDELEGATINGFUTURE_H_

#include <assert.h>
#include <boost/enable_shared_from_this.hpp>

#include <hazelcast/client/spi/InternalCompletableFuture.h>
#include <hazelcast/client/spi/impl/ClientInvocationFuture.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>
#include <hazelcast/client/impl/ClientMessageDecoder.h>
#include <hazelcast/client/spi/impl/ClientInvocation.h>

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace util {
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
                          public boost::enable_shared_from_this<ClientDelegatingFuture<V> > {
                public:
                    ClientDelegatingFuture(
                            const boost::shared_ptr<spi::impl::ClientInvocationFuture> &clientInvocationFuture,
                            serialization::pimpl::SerializationService &serializationService,
                            const boost::shared_ptr<impl::ClientMessageDecoder> &clientMessageDecoder,
                            const boost::shared_ptr<V> &defaultValue) : future(
                            clientInvocationFuture), serializationService(serializationService), clientMessageDecoder(
                            clientMessageDecoder), defaultValue(defaultValue), decodedResponse(VOIDOBJ) {
                        const boost::shared_ptr<spi::impl::ClientInvocation> invocation = clientInvocationFuture->getInvocation();
                        if (invocation.get()) {
                            userExecutor = invocation->getUserExecutor();
                        }
                    }

                    virtual void andThen(const boost::shared_ptr<ExecutionCallback<V> > &callback) {
                        future->andThen(boost::shared_ptr<ExecutionCallback<protocol::ClientMessage> >(
                                new DelegatingExecutionCallback(
                                        boost::enable_shared_from_this<ClientDelegatingFuture<V> >::shared_from_this(),
                                        callback)), userExecutor);
                    }

                    virtual void andThen(const boost::shared_ptr<ExecutionCallback<V> > &callback,
                                         const boost::shared_ptr<Executor> &executor) {
                        future->andThen(boost::shared_ptr<ExecutionCallback<protocol::ClientMessage> >(
                                new DelegatingExecutionCallback(
                                        boost::enable_shared_from_this<ClientDelegatingFuture<V> >::shared_from_this(),
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

                    virtual boost::shared_ptr<V> get() {
                        return get(INT64_MAX, hazelcast::util::concurrent::TimeUnit::MILLISECONDS());
                    }

                    virtual boost::shared_ptr<V> get(int64_t timeout, const TimeUnit &unit) {
                        boost::shared_ptr<protocol::ClientMessage> response = future->get(timeout, unit);
                        return resolveResponse(response);
                    }

                    virtual bool complete(const boost::shared_ptr<V> &value) {
                        assert(0);
                        return false;
                    }

                    virtual bool complete(const boost::shared_ptr<exception::IException> &value) {
                        return future->complete(value);
                    }

                protected:
                    const boost::shared_ptr<spi::impl::ClientInvocationFuture> &getFuture() const {
                        return future;
                    }

                private:
                    class DelegatingExecutionCallback : public ExecutionCallback<protocol::ClientMessage> {
                    public:
                        DelegatingExecutionCallback(
                                const boost::shared_ptr<ClientDelegatingFuture<V> > &delegatingFuture,
                                const boost::shared_ptr<ExecutionCallback<V> > &callback) : delegatingFuture(
                                delegatingFuture), callback(callback) {}

                        virtual void onResponse(const boost::shared_ptr<protocol::ClientMessage> &message) {
                            boost::shared_ptr<V> response = delegatingFuture->resolveResponse(message);
                            callback->onResponse(response);
                        }

                        virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                            callback->onFailure(e);
                        }

                    private:
                        boost::shared_ptr<ClientDelegatingFuture> delegatingFuture;
                        const boost::shared_ptr<ExecutionCallback<V> > callback;
                    };

                    boost::shared_ptr<V>
                    decodeResponse(const boost::shared_ptr<protocol::ClientMessage> &clientMessage) {
                        if (decodedResponse != VOIDOBJ) {
                            return decodedResponse;
                        }
                        // TODO: Java uses a message wrapper here --> ClientMessage message = ClientMessage.createForDecode(clientMessage.buffer(), 0);
                        boost::shared_ptr<serialization::pimpl::Data> newDecodedResponseData = clientMessageDecoder->decodeClientMessage(
                                clientMessage);

                        boost::shared_ptr<V> newDecodedResponse(
                                serializationService.toObject<V>(newDecodedResponseData.get()));
                        decodedResponse.compareAndSet(VOIDOBJ, newDecodedResponse);
                        return newDecodedResponse;
                    }

                    /* TODO: Java client does deserialization inside this method, do we need it ? */
                    boost::shared_ptr<V>
                    resolveResponse(const boost::shared_ptr<protocol::ClientMessage> &clientMessage) {
                        if (defaultValue.get() != NULL) {
                            return defaultValue;
                        }

                        return decodeResponse(clientMessage);
                    }

                    static const boost::shared_ptr<V> VOIDOBJ;
                    const boost::shared_ptr<spi::impl::ClientInvocationFuture> future;
                    serialization::pimpl::SerializationService &serializationService;
                    const boost::shared_ptr<impl::ClientMessageDecoder> clientMessageDecoder;
                    const boost::shared_ptr<V> defaultValue;
                    boost::shared_ptr<hazelcast::util::Executor> userExecutor;
                    hazelcast::util::Atomic<boost::shared_ptr<V> > decodedResponse;
                };

                template<typename V>
                const boost::shared_ptr<V> ClientDelegatingFuture<V>::VOIDOBJ(new V);
            }
        }
    }
};

#endif /* HAZELCAST_CLIENT_INTERNAL_UTIL_CLIENTDELEGATINGFUTURE_H_ */
