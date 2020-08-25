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

#include "hazelcast/client/ringbuffer/ReadResultSet.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/proxy/PartitionSpecificClientProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4250) //for warning class1' : inherits 'class2::member' via dominance
#pragma warning(disable: 4251)
#endif

namespace hazelcast {
    namespace client {
        namespace ringbuffer {
            /**
             * Using this policy one can control the behavior what should to be done when an item is about to be added to the ringbuffer,
             * but there is 0 remaining capacity.
             *
             * Overflowing happens when a time-to-live is set and the oldest item in the ringbuffer (the head) is not old enough to expire.
             *
             * @see Ringbuffer#addAsync(const E &, OverflowPolicy)
             * @see Ringbuffer#addAllAsync(const std::vector<E> &, OverflowPolicy)
             */
            enum struct OverflowPolicy {

                /**
                 * Using this policy the oldest item is overwritten no matter it is not old enough to retire. Using this policy you are
                 * sacrificing the time-to-live in favor of being able to write.
                 *
                 * Example: if there is a time-to-live of 30 seconds, the buffer is full and the oldest item in the ring has been placed a
                 * second ago, then there are 29 seconds remaining for that item. Using this policy you are going to overwrite no matter
                 * what.
                 */
                OVERWRITE = 0,

                /**
                 * Using this policy the call will fail immediately and the oldest item will not be overwritten before it is old enough
                 * to retire. So this policy sacrificing the ability to write in favor of time-to-live.
                 *
                 * The advantage of fail is that the caller can decide what to do since it doesn't trap the thread due to backoff.
                 *
                 * Example: if there is a time-to-live of 30 seconds, the buffer is full and the oldest item in the ring has been placed a
                 * second ago, then there are 29 seconds remaining for that item. Using this policy you are not going to overwrite that
                 * item for the next 29 seconds.
                 */
                FAIL = 1
            };
        }

        namespace proxy {
            class HAZELCAST_API RingbufferImpl : public PartitionSpecificClientProxy {
            public:
                static constexpr const char *SERVICE_NAME = "hz:impl:ringbufferService";;

                /**
                 * The maximum number of items that can be retrieved in 1 go using the {@link #readManyAsync(long, int, int, IFunction)}
                 * method.
                 */
                static constexpr int32_t MAX_BATCH_SIZE = 1000;

                /**
                 * Returns the capacity of this Ringbuffer.
                 *
                 * @return the capacity.
                 */
                boost::shared_future<int64_t> capacity() {
                    if (!bufferCapacity.valid()) {
                        auto request = protocol::codec::ringbuffer_capacity_encode(getName());
                        bufferCapacity = invokeAndGetFuture<int64_t>(request, partitionId).share();
                    }
                    return bufferCapacity;
                }

                /**
                 * Returns number of items in the ringbuffer.
                 *
                 * If no ttl is set, the size will always be equal to capacity after the head completed the first loop
                 * around the ring. This is because no items are getting retired.
                 *
                 * @return the size.
                 */
                boost::future<int64_t> size() {
                    auto request = protocol::codec::ringbuffer_size_encode(getName());
                    return invokeAndGetFuture<int64_t>(
                            request, partitionId);
                }

                /**
                 * Returns the sequence of the tail. The tail is the side of the ringbuffer where the items are added to.
                 *
                 * The initial value of the tail is -1.
                 *
                 * @return the sequence of the tail.
                 */
                boost::future<int64_t> tailSequence() {
                    auto request = protocol::codec::ringbuffer_tailsequence_encode(getName());
                    return invokeAndGetFuture<int64_t>(
                            request, partitionId);
                }

                /**
                 * Returns the sequence of the head. The head is the side of the ringbuffer where the oldest items in the
                 * ringbuffer are found.
                 *
                 * If the RingBuffer is empty, the head will be one more than the tail.
                 *
                 * The initial value of the head is 0 (1 more than tail).
                 *
                 * @return the sequence of the head.
                 */
                boost::future<int64_t> headSequence() {
                    auto request = protocol::codec::ringbuffer_headsequence_encode(getName());
                    return invokeAndGetFuture<int64_t>(
                            request, partitionId);
                }

                /**
                 * Returns the remaining capacity of the ringbuffer.
                 *
                 * The returned value could be stale as soon as it is returned.
                 *
                 * If ttl is not set, the remaining capacity will always be the capacity.
                 *
                 * @return the remaining capacity.
                 */
                boost::future<int64_t> remainingCapacity() {
                    auto request = protocol::codec::ringbuffer_remainingcapacity_encode(getName());
                    return invokeAndGetFuture<int64_t>(
                            request, partitionId);
                }

            protected:
                RingbufferImpl(const std::string &objectName, spi::ClientContext *context)
                        : PartitionSpecificClientProxy(SERVICE_NAME, objectName, context) {}

                boost::future<int64_t> addData(serialization::pimpl::Data &&itemData) {
                    auto request = protocol::codec::ringbuffer_add_encode(getName(),
                                                                                  static_cast<int32_t>(ringbuffer::OverflowPolicy::OVERWRITE),
                                                                                  itemData);
                    return invokeAndGetFuture<int64_t>(
                            request, partitionId);
                }

                boost::future<int64_t> addData(serialization::pimpl::Data &&itemData, ringbuffer::OverflowPolicy policy) {
                    auto request = protocol::codec::ringbuffer_add_encode(getName(),
                                                                                  static_cast<int32_t>(policy),
                                                                                  itemData);
                    return invokeAndGetFuture<int64_t>(
                            request, partitionId);
                }

                boost::future<boost::optional<serialization::pimpl::Data>>readOneData(int64_t sequence) {
                    checkSequence(sequence);
                    auto request = protocol::codec::ringbuffer_readone_encode(getName(), sequence);
                    return invokeAndGetFuture<boost::optional<serialization::pimpl::Data>>(
                            request, partitionId);
                }

                boost::future<int64_t>
                addAllData(std::vector<serialization::pimpl::Data> &&items, ringbuffer::OverflowPolicy overflowPolicy) {
                    util::Preconditions::checkNotEmpty(items, "items can't be empty");
                    util::Preconditions::checkMax((int32_t) items.size(), MAX_BATCH_SIZE, "items");

                    auto request = protocol::codec::ringbuffer_addall_encode(name, items,
                                                                             static_cast<int32_t>(overflowPolicy));
                    return invokeAndGetFuture<int64_t>(request, partitionId);
                }

            protected:
                boost::future<protocol::ClientMessage>
                readManyData(int64_t startSequence, int32_t minCount, int32_t maxCount,
                             serialization::pimpl::Data *filterData) {
                    checkSequence(startSequence);
                    util::Preconditions::checkNotNegative(minCount, "minCount can't be smaller than 0");
                    util::Preconditions::checkTrue(maxCount >= minCount,
                                                   "maxCount should be equal or larger than minCount");
                    try {
                        capacity().get();
                    } catch (exception::IException &) {
                        //in case of exception return the exception via future to behave consistently to member
                        try {
                            std::throw_with_nested(boost::enable_current_exception(
                                    exception::ExecutionException("ClientRingbufferProxy::readManyData",
                                                                  "capacity() method failed")));
                        } catch (...) {
                            return boost::make_exceptional_future<protocol::ClientMessage>(std::current_exception());
                        }
                    }

                    util::Preconditions::checkTrue(maxCount <= bufferCapacity.get(),
                                                   "the maxCount should be smaller than or equal to the capacity");
                    util::Preconditions::checkMax(maxCount, RingbufferImpl::MAX_BATCH_SIZE, "maxCount");

                    auto request = protocol::codec::ringbuffer_readmany_encode(
                            name,
                            startSequence,
                            minCount,
                            maxCount,
                            filterData);

                    return invokeOnPartition(request, partitionId);
                }

            private:
                boost::shared_future<int64_t> bufferCapacity;

                static void checkSequence(int64_t sequence) {
                    if (sequence < 0) {
                        throw (exception::ExceptionBuilder<exception::IllegalArgumentException>(
                                "ClientRingbufferProxy::checkSequence") << "sequence can't be smaller than 0, but was: "
                                                                        << sequence).build();
                    }
                }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

