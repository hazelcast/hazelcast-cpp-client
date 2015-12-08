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
#include "hazelcast/client/IAtomicLong.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/AtomicLongApplyCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongAlterCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongAlterAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndAlterCodec.h"
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

        long IAtomicLong::addAndGet(long delta) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongAddAndGetCodec::RequestParameters::encode(getName(), delta);

            return invokeAndGetResult<long, protocol::codec::AtomicLongAddAndGetCodec::ResponseParameters>(request, partitionId);
        }

        bool IAtomicLong::compareAndSet(long expect, long update) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongCompareAndSetCodec::RequestParameters::encode(getName(), expect, update);

            return invokeAndGetResult<bool, protocol::codec::AtomicLongCompareAndSetCodec::ResponseParameters>(request, partitionId);
        }

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        }

        long IAtomicLong::get() {
            return getAndAdd(0);
        }

        long IAtomicLong::getAndAdd(long delta) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndAddCodec::RequestParameters::encode(getName(), delta);

            return invokeAndGetResult<long, protocol::codec::AtomicLongGetAndAddCodec::ResponseParameters>(request, partitionId);
        }

        long IAtomicLong::getAndSet(long newValue) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndSetCodec::RequestParameters::encode(getName(), newValue);

            return invokeAndGetResult<long, protocol::codec::AtomicLongGetAndSetCodec::ResponseParameters>(request, partitionId);
        }

        long IAtomicLong::incrementAndGet() {
            return addAndGet(1);
        }

        long IAtomicLong::getAndIncrement() {
            return getAndAdd(1);
        }

        void IAtomicLong::set(long newValue) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongSetCodec::RequestParameters::encode(getName(), newValue);

            invoke(request, partitionId);
        }
    }
}
