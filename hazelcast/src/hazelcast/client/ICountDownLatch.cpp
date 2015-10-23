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
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/countdownlatch/AwaitRequest.h"
#include "hazelcast/client/countdownlatch/CountDownRequest.h"
#include "hazelcast/client/countdownlatch/GetCountRequest.h"
#include "hazelcast/client/countdownlatch/SetCountRequest.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        bool ICountDownLatch::await(long timeoutInMillis) {
            countdownlatch::AwaitRequest *request = new countdownlatch::AwaitRequest(getName(), timeoutInMillis);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, bool);
            return *result;
        }

        void ICountDownLatch::countDown() {
            countdownlatch::CountDownRequest *request = new countdownlatch::CountDownRequest(getName());
            invoke(request, partitionId);
        }

        int ICountDownLatch::getCount() {
            countdownlatch::GetCountRequest *request = new countdownlatch::GetCountRequest(getName());
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, int);
            return *result;
        }

        bool ICountDownLatch::trySetCount(int count) {
            countdownlatch::SetCountRequest *request = new countdownlatch::SetCountRequest(getName(), count);
            serialization::pimpl::Data data = invoke(request, partitionId);
            DESERIALIZE(data, bool);
            return *result;
        }
    }
}
