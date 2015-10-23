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
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/semaphore/InitRequest.h"
#include "hazelcast/client/semaphore/AcquireRequest.h"
#include "hazelcast/client/semaphore/AvailableRequest.h"
#include "hazelcast/client/semaphore/DrainRequest.h"
#include "hazelcast/client/semaphore/ReduceRequest.h"
#include "hazelcast/client/semaphore/ReleaseRequest.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string &name, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:semaphoreService", name, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&name);
            partitionId = getPartitionId(keyData);
        }

        bool ISemaphore::init(int permits) {
            semaphore::InitRequest *request = new semaphore::InitRequest(getName(), permits);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, bool);
            return *result;
        }

        void ISemaphore::acquire() {
            acquire(1);
        }

        void ISemaphore::acquire(int permits) {
            semaphore::AcquireRequest *request = new semaphore::AcquireRequest(getName(), permits, -1);
            invoke(request, partitionId);
        }

        int ISemaphore::availablePermits() {
            semaphore::AvailableRequest *request = new semaphore::AvailableRequest(getName());
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, int);
            return *result;
        }

        int ISemaphore::drainPermits() {
            semaphore::DrainRequest *request = new semaphore::DrainRequest(getName());
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, int);
            return *result;
        }

        void ISemaphore::reducePermits(int reduction) {
            semaphore::ReduceRequest *request = new semaphore::ReduceRequest(getName(), reduction);
            invoke(request, partitionId);
        }

        void ISemaphore::release() {
            release(1);
        }

        void ISemaphore::release(int permits) {
            semaphore::ReleaseRequest *request = new semaphore::ReleaseRequest(getName(), permits);
            invoke(request, partitionId);
        }

        bool ISemaphore::tryAcquire() {
            return tryAcquire(1);
        }

        bool ISemaphore::tryAcquire(int permits) {
                return tryAcquire(permits, 0);
        }

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        }

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            semaphore::AcquireRequest *request = new semaphore::AcquireRequest(getName(), permits, timeoutInMillis);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, bool);
            return *result;
        }
    }
}
