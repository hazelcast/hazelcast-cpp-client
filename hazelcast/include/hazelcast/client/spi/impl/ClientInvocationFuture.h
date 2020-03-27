/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/spi/impl/AbstractInvocationFuture.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                class ClientInvocation;

                namespace sequence {
                    class CallIdSequence;
                }

                class HAZELCAST_API ClientInvocationFuture : public AbstractInvocationFuture<protocol::ClientMessage> {
                    friend class ClientInvocation;
                public:
                    ClientInvocationFuture(const std::shared_ptr<Executor> &defaultExecutor, ILogger &logger,
                                           const std::shared_ptr<protocol::ClientMessage> &request,
                                           const std::shared_ptr<sequence::CallIdSequence> &callIdSequence);

                    virtual void andThen(const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback,
                                         const std::shared_ptr<Executor> &executor);

                    virtual void
                    andThen(const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback);

                    const std::shared_ptr<ClientInvocation> getInvocation() const;

                protected:
                    virtual std::string invocationToString() const;

                    virtual void onComplete();

                    virtual std::shared_ptr<protocol::ClientMessage>
                    resolveAndThrowIfException(
                            const std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::BaseState> &response);

                private:
                    class InternalDelegatingExecutionCallback
                            : public client::ExecutionCallback<protocol::ClientMessage> {
                    public:
                        InternalDelegatingExecutionCallback(
                                const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback,
                                const std::shared_ptr<sequence::CallIdSequence> &callIdSequence);

                        virtual void onResponse(const std::shared_ptr<protocol::ClientMessage> &message);

                        virtual void onFailure(const std::shared_ptr<exception::IException> &e);

                    private:
                        const std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> > callback;
                        const std::shared_ptr<sequence::CallIdSequence> callIdSequence;
                    };

                    void setInvocation(const std::weak_ptr<spi::impl::ClientInvocation> &invocation);

                    std::weak_ptr<spi::impl::ClientInvocation> invocation;
                    const std::shared_ptr<protocol::ClientMessage> request;
                    const std::shared_ptr<sequence::CallIdSequence> callIdSequence;
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_
