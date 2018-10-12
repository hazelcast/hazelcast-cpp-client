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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/impl/ExecutionCallback.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                class HAZELCAST_API ClientInvocationFuture
                        : public util::Future<boost::shared_ptr<protocol::ClientMessage> > {
                public:
                    ClientInvocationFuture(util::ILogger &logger)
                        : util::Future<boost::shared_ptr<protocol::ClientMessage> >(logger) {}

                    virtual void
                    andThen(const boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > > &callback) = 0;

                    virtual std::string invocationToString() = 0;
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_
