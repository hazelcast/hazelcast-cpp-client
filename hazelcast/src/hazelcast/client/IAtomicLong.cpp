/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/IAtomicLong.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/AtomicLongAddAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongCompareAndSetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongDecrementAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndAddCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndSetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongIncrementAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndIncrementCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongSetCodec.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {
        IAtomicLong::IAtomicLong(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        int64_t IAtomicLong::addAndGet(int64_t delta) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongAddAndGetCodec::RequestParameters::encode(getName(), delta);

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongAddAndGetCodec::ResponseParameters>(request, partitionId);
        }

        bool IAtomicLong::compareAndSet(int64_t expect, int64_t update) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongCompareAndSetCodec::RequestParameters::encode(getName(), expect, update);

            return invokeAndGetResult<bool, protocol::codec::AtomicLongCompareAndSetCodec::ResponseParameters>(request, partitionId);
        }

        int64_t IAtomicLong::decrementAndGet() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongDecrementAndGetCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongDecrementAndGetCodec::ResponseParameters>(request, partitionId);
        }

        int64_t IAtomicLong::get() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetCodec::ResponseParameters>(request, partitionId);
        }

        int64_t IAtomicLong::getAndAdd(int64_t delta) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndAddCodec::RequestParameters::encode(getName(), delta);

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndAddCodec::ResponseParameters>(request, partitionId);
        }

        int64_t IAtomicLong::getAndSet(int64_t newValue) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndSetCodec::RequestParameters::encode(getName(), newValue);

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndSetCodec::ResponseParameters>(request, partitionId);
        }

        int64_t IAtomicLong::incrementAndGet() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongIncrementAndGetCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongIncrementAndGetCodec::ResponseParameters>(request, partitionId);
        }

        int64_t IAtomicLong::getAndIncrement() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndIncrementCodec::RequestParameters::encode(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndIncrementCodec::ResponseParameters>(request, partitionId);
        }

        void IAtomicLong::set(int64_t newValue) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongSetCodec::RequestParameters::encode(getName(), newValue);

            invoke(request, partitionId);
        }
    }
}
