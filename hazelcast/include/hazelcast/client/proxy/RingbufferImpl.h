/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/proxy/PartitionSpecificClientProxy.h"
#include "hazelcast/util/Preconditions.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4250) // for warning class1' : inherits
                                // 'class2::member' via dominance
#pragma warning(disable : 4251)
#endif

namespace hazelcast {
namespace client {
namespace rb {
/**
 * Using this policy one can control the behavior what should to be done when an
 * item is about to be added to the ringbuffer, but there is 0 remaining
 * capacity.
 *
 * Overflowing happens when a time-to-live is set and the oldest item in the
 * ringbuffer (the head) is not old enough to expire.
 *
 * @see Ringbuffer#addAsync(const E &, OverflowPolicy)
 * @see Ringbuffer#add_allAsync(const std::vector<E> &, OverflowPolicy)
 */
enum struct overflow_policy
{

    /**
     * Using this policy the oldest item is overwritten no matter it is not old
     * enough to retire. Using this policy you are sacrificing the time-to-live
     * in favor of being able to write.
     *
     * Example: if there is a time-to-live of 30 seconds, the buffer is full and
     * the oldest item in the ring has been placed a second ago, then there are
     * 29 seconds remaining for that item. Using this policy you are going to
     * overwrite no matter what.
     */
    OVERWRITE = 0,

    /**
     * Using this policy the call will fail immediately and the oldest item will
     * not be overwritten before it is old enough to retire. So this policy
     * sacrificing the ability to write in favor of time-to-live.
     *
     * The advantage of fail is that the caller can decide what to do since it
     * doesn't trap the thread due to backoff.
     *
     * Example: if there is a time-to-live of 30 seconds, the buffer is full and
     * the oldest item in the ring has been placed a second ago, then there are
     * 29 seconds remaining for that item. Using this policy you are not going
     * to overwrite that item for the next 29 seconds.
     */
    FAIL = 1
};
} // namespace rb

namespace proxy {
class HAZELCAST_API RingbufferImpl : public PartitionSpecificClientProxy
{
public:
    static constexpr const char* SERVICE_NAME = "hz:impl:ringbufferService";
    ;

    /**
     * The maximum number of items that can be retrieved in 1 go using the
     * {@link #readManyAsync(long, int, int, IFunction)} method.
     */
    static const int32_t MAX_BATCH_SIZE;

    /**
     * Returns the capacity of this Ringbuffer.
     *
     * @return the capacity.
     */
    boost::shared_future<int64_t> capacity()
    {
        if (!buffer_capacity_.valid()) {
            auto request =
              protocol::codec::ringbuffer_capacity_encode(get_name());
            buffer_capacity_ =
              invoke_and_get_future<int64_t>(request, partition_id_).share();
        }
        return buffer_capacity_;
    }

    /**
     * Returns number of items in the ringbuffer.
     *
     * If no ttl is set, the size will always be equal to capacity after the
     * head completed the first loop around the ring. This is because no items
     * are getting retired.
     *
     * @return the size.
     */
    boost::future<int64_t> size()
    {
        auto request = protocol::codec::ringbuffer_size_encode(get_name());
        return invoke_and_get_future<int64_t>(request, partition_id_);
    }

    /**
     * Returns the sequence of the tail. The tail is the side of the ringbuffer
     * where the items are added to.
     *
     * The initial value of the tail is -1.
     *
     * @return the sequence of the tail.
     */
    boost::future<int64_t> tail_sequence()
    {
        auto request =
          protocol::codec::ringbuffer_tailsequence_encode(get_name());
        return invoke_and_get_future<int64_t>(request, partition_id_);
    }

    /**
     * Returns the sequence of the head. The head is the side of the ringbuffer
     * where the oldest items in the ringbuffer are found.
     *
     * If the RingBuffer is empty, the head will be one more than the tail.
     *
     * The initial value of the head is 0 (1 more than tail).
     *
     * @return the sequence of the head.
     */
    boost::future<int64_t> head_sequence()
    {
        auto request =
          protocol::codec::ringbuffer_headsequence_encode(get_name());
        return invoke_and_get_future<int64_t>(request, partition_id_);
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
    boost::future<int64_t> remaining_capacity()
    {
        auto request =
          protocol::codec::ringbuffer_remainingcapacity_encode(get_name());
        return invoke_and_get_future<int64_t>(request, partition_id_);
    }

protected:
    RingbufferImpl(const std::string& object_name, spi::ClientContext* context)
      : PartitionSpecificClientProxy(SERVICE_NAME, object_name, context)
    {}

    boost::future<int64_t> add_data(serialization::pimpl::data&& item_data)
    {
        auto request = protocol::codec::ringbuffer_add_encode(
          get_name(),
          static_cast<int32_t>(rb::overflow_policy::OVERWRITE),
          item_data);
        return invoke_and_get_future<int64_t>(request, partition_id_);
    }

    boost::future<int64_t> add_data(serialization::pimpl::data&& item_data,
                                    rb::overflow_policy policy)
    {
        auto request = protocol::codec::ringbuffer_add_encode(
          get_name(), static_cast<int32_t>(policy), item_data);
        return invoke_and_get_future<int64_t>(request, partition_id_);
    }

    boost::future<boost::optional<serialization::pimpl::data>> read_one_data(
      int64_t sequence)
    {
        check_sequence(sequence);
        auto request =
          protocol::codec::ringbuffer_readone_encode(get_name(), sequence);
        return invoke_and_get_future<
          boost::optional<serialization::pimpl::data>>(request, partition_id_);
    }

    boost::future<int64_t> add_all_data(
      std::vector<serialization::pimpl::data>&& items,
      rb::overflow_policy overflow_policy)
    {
        util::Preconditions::check_not_empty(items, "items can't be empty");
        util::Preconditions::check_max(
          (int32_t)items.size(), MAX_BATCH_SIZE, "items");

        auto request = protocol::codec::ringbuffer_addall_encode(
          name_, items, static_cast<int32_t>(overflow_policy));
        return invoke_and_get_future<int64_t>(request, partition_id_);
    }

protected:
    boost::future<protocol::ClientMessage> read_many_data(
      int64_t start_sequence,
      int32_t min_count,
      int32_t max_count,
      serialization::pimpl::data* filter_data)
    {
        check_sequence(start_sequence);
        util::Preconditions::check_not_negative(
          min_count,
          (boost::format("min_count (%1%) can't be smaller than 0") % min_count)
            .str());
        util::Preconditions::check_true(
          max_count >= min_count,
          (boost::format(
             "max_count (%1%) should be equal or larger than min_count(%2%)") %
           max_count % min_count)
            .str());
        try {
            capacity().get();
        } catch (exception::iexception&) {
            // in case of exception return the exception via future to behave
            // consistently to member
            try {
                std::throw_with_nested(boost::enable_current_exception(
                  exception::execution("ClientRingbufferProxy::readManyData",
                                       "capacity() method failed")));
            } catch (...) {
                return boost::make_exceptional_future<protocol::ClientMessage>(
                  std::current_exception());
            }
        }

        auto capacity = buffer_capacity_.get();
        util::Preconditions::check_true(
          max_count <= capacity,
          (boost::format("the max_count(%1%) should be smaller than or equal "
                         "to the capacity(%2%)") %
           max_count % capacity)
            .str());
        util::Preconditions::check_max(
          max_count, RingbufferImpl::MAX_BATCH_SIZE, "max_count");

        auto request = protocol::codec::ringbuffer_readmany_encode(
          name_, start_sequence, min_count, max_count, filter_data);

        return invoke_on_partition(request, partition_id_);
    }

private:
    boost::shared_future<int64_t> buffer_capacity_;

    static void check_sequence(int64_t sequence)
    {
        if (sequence < 0) {
            throw(exception::exception_builder<exception::illegal_argument>(
                    "ClientRingbufferProxy::checkSequence")
                  << "sequence can't be smaller than 0, but was: " << sequence)
              .build();
        }
    }
};
} // namespace proxy
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
