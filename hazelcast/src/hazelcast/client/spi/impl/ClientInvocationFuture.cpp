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

#include <hazelcast/client/spi/impl/ClientInvocationFuture.h>

#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                ClientInvocationFuture::ClientInvocationFuture(
                        const std::shared_ptr<Executor> &defaultExecutor, ILogger &logger,
                        const std::shared_ptr<protocol::ClientMessage> &request,
                        const std::shared_ptr<sequence::CallIdSequence> &callIdSequence)
                        : AbstractInvocationFuture<protocol::ClientMessage>(defaultExecutor, logger), request(request),
                          callIdSequence(callIdSequence) {}

                std::string ClientInvocationFuture::invocationToString() const {
                    std::ostringstream out;
                    out << *request;
                    return out.str();
                }

                void ClientInvocationFuture::andThen(
                        const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback,
                        const std::shared_ptr<Executor> &executor) {
                    AbstractInvocationFuture<protocol::ClientMessage>::andThen(
                            std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> >(
                                    new InternalDelegatingExecutionCallback(callback, callIdSequence)), executor);
                }

                void ClientInvocationFuture::andThen(
                        const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback) {
                    AbstractInvocationFuture<protocol::ClientMessage>::andThen(callback);
                }

                void ClientInvocationFuture::onComplete() {
                    callIdSequence->complete();
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocationFuture::resolveAndThrowIfException(
                        const std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::BaseState> &response) {
                    if (response->getType() == BaseState::Exception) {
                        std::shared_ptr<ExceptionState> exceptionState = std::static_pointer_cast<ExceptionState>(
                                response);
                        std::shared_ptr<IException> exception = exceptionState->getException();
                        int32_t errorCode = exception->getErrorCode();
                        if (errorCode == ExecutionException::ERROR_CODE || errorCode == protocol::CANCELLATION ||
                            errorCode == protocol::INTERRUPTED) {
                            exception->raise();
                        }

                        throw ExecutionException("ClientInvocationFuture::resolveAndThrowIfException",
                                                 "ExecutionException occured.", exception);
                    }
                    std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::ValueState> valueState =
                            std::static_pointer_cast<AbstractInvocationFuture<protocol::ClientMessage>::ValueState>(
                                    response);

                    return valueState->getValue();
                }

                const std::shared_ptr<ClientInvocation> ClientInvocationFuture::getInvocation() {
                    return invocation.lock();
                }

                void ClientInvocationFuture::setInvocation(
                        const std::weak_ptr<spi::impl::ClientInvocation> &invocation) {
                    this->invocation = invocation;
                }

                ClientInvocationFuture::InternalDelegatingExecutionCallback::InternalDelegatingExecutionCallback(
                        const std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> > &callback,
                        const std::shared_ptr<sequence::CallIdSequence> &callIdSequence) : callback(callback),
                                                                                             callIdSequence(
                                                                                                     callIdSequence) {
                    this->callIdSequence->forceNext();
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onResponse(
                        const std::shared_ptr<protocol::ClientMessage> &message) {
                    try {
                        callback->onResponse(message);
                        callIdSequence->complete();
                    } catch (...) {
                        callIdSequence->complete();
                    }
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onFailure(
                        const std::shared_ptr<exception::IException> &e) {
                    try {
                        callback->onFailure(e);
                        callIdSequence->complete();
                    } catch (...) {
                        callIdSequence->complete();
                    }
                }

            }
        }
    }
}
