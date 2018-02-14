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
#include "hazelcast/client/ISemaphore.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/SemaphoreInitCodec.h"
#include "hazelcast/client/protocol/codec/SemaphoreAcquireCodec.h"
#include "hazelcast/client/protocol/codec/SemaphoreAvailablePermitsCodec.h"
#include "hazelcast/client/protocol/codec/SemaphoreDrainPermitsCodec.h"
#include "hazelcast/client/protocol/codec/SemaphoreReducePermitsCodec.h"
#include "hazelcast/client/protocol/codec/SemaphoreReleaseCodec.h"
#include "hazelcast/client/protocol/codec/SemaphoreTryAcquireCodec.h"

namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string &name, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:semaphoreService", name, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&name);
            partitionId = getPartitionId(keyData);
        }

        bool ISemaphore::init(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreInitCodec::RequestParameters::encode(getName(), permits);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreInitCodec::ResponseParameters>(request, partitionId);
        }

        void ISemaphore::acquire() {
            acquire(1);
        }

        void ISemaphore::acquire(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreAcquireCodec::RequestParameters::encode(getName(), permits);

            invoke(request, partitionId);
        }

        int ISemaphore::availablePermits() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreAvailablePermitsCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int, protocol::codec::SemaphoreAvailablePermitsCodec::ResponseParameters>(request, partitionId);
        }

        int ISemaphore::drainPermits() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreDrainPermitsCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int, protocol::codec::SemaphoreDrainPermitsCodec::ResponseParameters>(request, partitionId);
        }

        void ISemaphore::reducePermits(int reduction) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreReducePermitsCodec::RequestParameters::encode(getName(), reduction);

            invoke(request, partitionId);
        }

        void ISemaphore::release() {
            release(1);
        }

        void ISemaphore::release(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreReleaseCodec::RequestParameters::encode(getName(), permits);

            invoke(request, partitionId);
        }

        bool ISemaphore::tryAcquire() {
            return tryAcquire(int(1));
        }

        bool ISemaphore::tryAcquire(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreTryAcquireCodec::RequestParameters::encode(getName(), permits, 0);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreTryAcquireCodec::ResponseParameters>(request, partitionId);
        }

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        }

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::SemaphoreTryAcquireCodec::RequestParameters::encode(getName(), permits, timeoutInMillis);

            return invokeAndGetResult<bool, protocol::codec::SemaphoreTryAcquireCodec::ResponseParameters>(request, partitionId);
        }
    }
}
