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
#include "hazelcast/client/ICountDownLatch.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/CountDownLatchAwaitCodec.h"
#include "hazelcast/client/protocol/codec/CountDownLatchCountDownCodec.h"
#include "hazelcast/client/protocol/codec/CountDownLatchGetCountCodec.h"
#include "hazelcast/client/protocol/codec/CountDownLatchTrySetCountCodec.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        bool ICountDownLatch::await(long timeoutInMillis) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchAwaitCodec::encodeRequest(getName(), timeoutInMillis);

            return invokeAndGetResult<bool, protocol::codec::CountDownLatchAwaitCodec::ResponseParameters>(request, partitionId);
        }

        void ICountDownLatch::countDown() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchCountDownCodec::encodeRequest(getName());

            invokeOnPartition(request, partitionId);
        }

        int ICountDownLatch::getCount() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchGetCountCodec::encodeRequest(getName());

            return invokeAndGetResult<int, protocol::codec::CountDownLatchGetCountCodec::ResponseParameters>(request, partitionId);
        }

        bool ICountDownLatch::trySetCount(int count) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::CountDownLatchTrySetCountCodec::encodeRequest(getName(), count);

            return invokeAndGetResult<bool, protocol::codec::CountDownLatchTrySetCountCodec::ResponseParameters>(request, partitionId);
        }
    }
}
