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


#pragma once

#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"
#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/client/topic/ReliableListener.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"
#include "hazelcast/client/config/ReliableTopicConfig.h"
#include "hazelcast/logger.h"

#include <memory>

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
                static constexpr const char * TOPIC_RB_PREFIX = "_hz_rb_";

                ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context);

                boost::future<void> publish(serialization::pimpl::Data &&data);
            protected:
                std::shared_ptr<Ringbuffer> ringbuffer;
                logger &logger_;
                const config::ReliableTopicConfig *config;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


