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

#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                ClientInvocationFuture::ClientInvocationFuture(util::ILogger &logger, ClientInvocation &invocation,
                                                               sequence::CallIdSequence &callIdSequence,
                                                               util::Executor &internalExecutor)
                        : util::Future<boost::shared_ptr<protocol::ClientMessage> >(logger),
                          invocation(invocation), callIdSequence(callIdSequence),
                          internalExecutor(internalExecutor) {}

                void ClientInvocationFuture::onComplete() {
                    callIdSequence.complete();
                }

                ClientInvocation &ClientInvocationFuture::getInvocation() const {
                    return invocation;
                }

                void
                ClientInvocationFuture::andThen(
                        const boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > > &callback) {
                    util::Future<boost::shared_ptr<protocol::ClientMessage> >::andThen(
                            boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > >(
                                    new InternalDelegatingExecutionCallback(callback, callIdSequence)),
                            internalExecutor);
                }

                std::string ClientInvocationFuture::invocationToString() {
                    std::ostringstream out;
                    out << invocation;
                    return out.str();
                }

                ClientInvocationFuture::InternalDelegatingExecutionCallback::InternalDelegatingExecutionCallback(
                        const boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > > &callback,
                        sequence::CallIdSequence &callIdSequence) : callback(callback),
                                                                    callIdSequence(callIdSequence) {
                    this->callIdSequence.forceNext();
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onResponse(
                        const boost::shared_ptr<protocol::ClientMessage> &message) {
                    try {
                        callback->onResponse(message);
                        callIdSequence.complete();
                    } catch (...) {
                        callIdSequence.complete();
                    }
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onFailure(
                        const boost::shared_ptr<exception::IException> &e) {
                    try {
                        callback->onFailure(e);
                        callIdSequence.complete();
                    } catch (...) {
                        callIdSequence.complete();
                    }
                }


            }
        }
    }
}
