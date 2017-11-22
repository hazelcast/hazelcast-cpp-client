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
#include "hazelcast/client/MixedRingbuffer.h"
#include "hazelcast/client/protocol/codec/RingbufferReadOneCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferReadManyCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferCapacityCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferSizeCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferTailSequenceCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferHeadSequenceCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferRemainingCapacityCodec.h"

namespace hazelcast {
    namespace client {
        MixedRingbuffer::MixedRingbuffer(const std::string &objectName, spi::ClientContext *context) : proxy::ProxyImpl(
                "hz:impl:ringbufferService", objectName, context), bufferCapacity(-1) {
            partitionId = getPartitionId(toData(objectName));
        }

        MixedRingbuffer::~MixedRingbuffer() {
        }

        int64_t MixedRingbuffer::capacity() {
            if (-1 == bufferCapacity) {
                std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferCapacityCodec::RequestParameters::encode(
                        getName());
                bufferCapacity = invokeAndGetResult<int64_t, protocol::codec::RingbufferCapacityCodec::ResponseParameters>(
                        msg, partitionId);
            }
            return bufferCapacity;
        }

        int64_t MixedRingbuffer::size() {
            std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferSizeCodec::RequestParameters::encode(
                    getName());
            return invokeAndGetResult<int64_t, protocol::codec::RingbufferSizeCodec::ResponseParameters>(msg, partitionId);
        }

        int64_t MixedRingbuffer::tailSequence() {
            std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferTailSequenceCodec::RequestParameters::encode(
                    getName());
            return invokeAndGetResult<int64_t, protocol::codec::RingbufferTailSequenceCodec::ResponseParameters>(
                    msg, partitionId);
        }

        int64_t MixedRingbuffer::headSequence() {
            std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferHeadSequenceCodec::RequestParameters::encode(
                    getName());
            return invokeAndGetResult<int64_t, protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters>(
                    msg, partitionId);
        }

        int64_t MixedRingbuffer::remainingCapacity() {
            std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferRemainingCapacityCodec::RequestParameters::encode(
                    getName());
            return invokeAndGetResult<int64_t, protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters>(
                    msg, partitionId);
        }

        TypedData MixedRingbuffer::readOne(int64_t sequence) {
            std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadOneCodec::RequestParameters::encode(
                    getName(), sequence);

            std::auto_ptr<serialization::pimpl::Data> itemData = invokeAndGetResult<
                    std::auto_ptr<serialization::pimpl::Data>, protocol::codec::RingbufferReadOneCodec::ResponseParameters>(
                    msg, partitionId);

            return TypedData(itemData, context->getSerializationService());
        }
    }
}
