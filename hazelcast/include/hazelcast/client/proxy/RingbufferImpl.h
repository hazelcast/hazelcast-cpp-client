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
//
// Created by ihsan demir on 26 May 2016.
//
#ifndef HAZELCAST_CLIENT_PROXY_RINGBUFFERIMPL_H_
#define HAZELCAST_CLIENT_PROXY_RINGBUFFERIMPL_H_

#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/protocol/codec/RingbufferReadManyCodec.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/client/impl/DataArrayImpl.h"
#include "hazelcast/client/protocol/codec/RingbufferReadOneCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferCapacityCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferSizeCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferTailSequenceCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferHeadSequenceCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferRemainingCapacityCodec.h"
#include "hazelcast/client/protocol/codec/RingbufferAddCodec.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/client/Ringbuffer.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace proxy {
            template<typename E>
            class RingbufferImpl : public Ringbuffer<E>, public ProxyImpl {
            public:
                RingbufferImpl(const std::string &objectName, spi::ClientContext *context) : ProxyImpl(
                        "hz:impl:ringbufferService", objectName, context), bufferCapacity(-1) {
                    partitionId = getPartitionId(toData(objectName));
                }

                virtual ~RingbufferImpl() {
                }

                /****************  RingBuffer<E> interface implementation starts here *********************************/
                int64_t capacity() {
                    if (-1 == bufferCapacity) {
                        std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferCapacityCodec::encodeRequest(
                                getName());
                        bufferCapacity = invokeAndGetResult<int64_t, protocol::codec::RingbufferCapacityCodec::ResponseParameters>(
                                msg, partitionId);
                    }
                    return bufferCapacity;
                }

                int64_t size() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferSizeCodec::encodeRequest(
                            getName());
                    return invokeAndGetResult<int64_t, protocol::codec::RingbufferSizeCodec::ResponseParameters>(msg, partitionId);
                }

                int64_t tailSequence() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferTailSequenceCodec::encodeRequest(
                            getName());
                    return invokeAndGetResult<int64_t, protocol::codec::RingbufferTailSequenceCodec::ResponseParameters>(
                            msg, partitionId);
                }

                int64_t headSequence() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferHeadSequenceCodec::encodeRequest(
                            getName());
                    return invokeAndGetResult<int64_t, protocol::codec::RingbufferHeadSequenceCodec::ResponseParameters>(
                            msg, partitionId);
                }

                int64_t remainingCapacity() {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferRemainingCapacityCodec::encodeRequest(
                            getName());
                    return invokeAndGetResult<int64_t, protocol::codec::RingbufferRemainingCapacityCodec::ResponseParameters>(
                            msg, partitionId);
                }

                int64_t add(const E &item) {
                    serialization::pimpl::Data itemData = toData<E>(item);
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferAddCodec::encodeRequest(
                            getName(), OVERWRITE, itemData);
                    return invokeAndGetResult<int64_t, protocol::codec::RingbufferAddCodec::ResponseParameters>(msg,
                                                                                                                partitionId);
                }

                std::auto_ptr<E> readOne(int64_t sequence) {
                    checkSequence(sequence);
                    
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::RingbufferReadOneCodec::encodeRequest(
                            getName(), sequence);

                    std::auto_ptr<serialization::pimpl::Data> itemData = invokeAndGetResult<
                            std::auto_ptr<serialization::pimpl::Data>, protocol::codec::RingbufferReadOneCodec::ResponseParameters>(
                            request, partitionId);

                    return toObject<E>(itemData);
                }

                boost::shared_ptr<spi::impl::ClientInvocationFuture>
                readManyAsync(int64_t sequence, int32_t maxCount, time_t timeoutSeconds) {
                    std::auto_ptr<protocol::ClientMessage> msg = protocol::codec::RingbufferReadManyCodec::encodeRequest(
                            getName(), sequence, 1, maxCount, (const serialization::pimpl::Data *) NULL);

                    return invokeAndGetFuture(msg, partitionId);
                }

                std::auto_ptr<DataArray<E> > getReadManyAsyncResponseObject(
                        boost::shared_ptr<protocol::ClientMessage> responseMsg) {
                    protocol::codec::RingbufferReadManyCodec::ResponseParameters responseParameters =
                            protocol::codec::RingbufferReadManyCodec::ResponseParameters::decode(*responseMsg);
                    return std::auto_ptr<DataArray<E> >(new impl::DataArrayImpl<E>(responseParameters.items,
                                                                                   getContext().getSerializationService()));
                }

                const std::string &getServiceName() const {
                    return ProxyImpl::getServiceName();
                }

                const std::string &getName() const {
                    return ProxyImpl::getName();
                }

                void destroy() {
                    ProxyImpl::destroy();
                }
                /***************** RingBuffer<E> interface implementation ends here ***********************************/
            protected:
                boost::shared_ptr<protocol::ClientMessage>
                invoke(std::auto_ptr<protocol::ClientMessage> &clientMessage, int32_t partitionId) {
                    try {
                        return invokeOnPartition(clientMessage, partitionId);
                    } catch (exception::ExecutionException &e) {
                        boost::shared_ptr<exception::IException> cause = e.getCause();
                        if (cause->getErrorCode() == exception::StaleSequenceException::ERROR_CODE) {
                            // can not use static_
                            boost::shared_ptr<exception::StaleSequenceException> se = boost::dynamic_pointer_cast<exception::StaleSequenceException>(
                                    cause);
                            int64_t l = headSequence();
                            throw (exception::ExceptionBuilder<exception::StaleSequenceException>(se->getSource())
                                    << se->getMessage() << ", head sequence:" << l).build();
                        }
                        util::ExceptionUtil::rethrow(e);
                    } catch (const exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<protocol::ClientMessage>();
                }

            private:
                /**
                 * Using this policy one can control the behavior what should to be done when an item is about to be added to the ringbuffer,
                 * but there is 0 remaining capacity.
                 *
                 * Overflowing happens when a time-to-live is set and the oldest item in the ringbuffer (the head) is not old enough to expire.
                 *
                 * @see Ringbuffer#addAsync(Object, OverflowPolicy)
                 * @see Ringbuffer#addAllAsync(java.util.Collection, OverflowPolicy)
                 */
                enum OverflowPolicy {

                    /**
                     * Using this policy the oldest item is overwritten no matter it is not old enough to retire. Using this policy you are
                     * sacrificing the time-to-live in favor of being able to write.
                     *
                     * Example: if there is a time-to-live of 30 seconds, the buffer is full and the oldest item in the ring has been placed a
                     * second ago, then there are 29 seconds remaining for that item. Using this policy you are going to overwrite no matter
                     * what.
                     */
                            OVERWRITE = 0
                };

                static void checkSequence(int64_t sequence) {
                    if (sequence < 0) {
                        throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                "RingbufferImpl::checkSequence") << "sequence can't be smaller than 0, but was: "
                                                                 << sequence).build();
                    }
                }

                template<typename T, typename CODEC>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> &request, int partitionId) {
                    boost::shared_ptr<protocol::ClientMessage> response = invoke(request, partitionId);

                    return (T)CODEC::decode(*response).response;
                }

                int32_t partitionId;
                util::Atomic<int64_t> bufferCapacity;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_PROXY_RINGBUFFERIMPL_H_

