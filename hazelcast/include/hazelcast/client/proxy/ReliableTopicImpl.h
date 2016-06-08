/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 01/10/14.
//

#ifndef HAZELCAST_CLIENT_PROXY_RELIBALETOPICIMPL_H_
#define HAZELCAST_CLIENT_PROXY_RELIBALETOPICIMPL_H_

#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"
#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/topic/ReliableMessageListener.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/impl/ExecutionCallback.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"

#include <string>
#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ReliableTopicImpl : public proxy::ProxyImpl {
            protected:
                ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context,
                                  std::auto_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb);

                void publish(const serialization::pimpl::Data &data);

            protected:
                std::auto_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > ringbuffer;
                util::ILogger &logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_PROXY_RELIBALETOPICIMPL_H_
