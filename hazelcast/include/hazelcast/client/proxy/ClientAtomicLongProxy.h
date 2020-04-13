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
#ifndef HAZELCAST_CLIENT_PROXY_CLIENTATOMICLONGPROXY_H_
#define HAZELCAST_CLIENT_PROXY_CLIENTATOMICLONGPROXY_H_

#include "hazelcast/client/proxy/PartitionSpecificClientProxy.h"
#include "hazelcast/client/impl/AtomicLongInterface.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            /**
             * Client proxy implementation for a {@link AtomicLongInterface}.
             */
            class ClientAtomicLongProxy : public impl::AtomicLongInterface, public proxy::PartitionSpecificClientProxy {
            public:
                static const std::string SERVICE_NAME;

                ClientAtomicLongProxy(const std::string& objectName, spi::ClientContext *context);

                virtual int64_t addAndGet(int64_t delta);

                virtual bool compareAndSet(int64_t expect, int64_t update);

                virtual int64_t decrementAndGet();

                virtual int64_t get();

                virtual int64_t getAndAdd(int64_t delta);

                virtual int64_t getAndSet(int64_t newValue);

                virtual int64_t incrementAndGet();

                virtual int64_t getAndIncrement();

                virtual void set(int64_t newValue);

                virtual future<std::shared_ptr<int64_t>> addAndGetAsync(int64_t delta);

                virtual future<std::shared_ptr<bool>> compareAndSetAsync(int64_t expect, int64_t update);

                virtual future<std::shared_ptr<int64_t>> decrementAndGetAsync();

                virtual future<std::shared_ptr<int64_t>> getAsync();

                virtual future<std::shared_ptr<int64_t>> getAndAddAsync(int64_t delta);

                virtual future<std::shared_ptr<int64_t>> getAndSetAsync(int64_t newValue);

                virtual future<std::shared_ptr<int64_t>> incrementAndGetAsync();

                virtual future<std::shared_ptr<int64_t>> getAndIncrementAsync();

                virtual future<void> setAsync(int64_t newValue);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_CLIENTATOMICLONGPROXY_H_
