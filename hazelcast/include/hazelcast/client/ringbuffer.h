/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/RingbufferImpl.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 * A Ringbuffer is a data-structure where the content is stored in a ring like
 * structure. A ringbuffer has a capacity so it won't grow beyond that capacity
 * and endanger the stability of the system. If that capacity is exceeded, than
 * the oldest item in the ringbuffer is overwritten.
 *
 * The ringbuffer has 2 always incrementing sequences:
 * <ol>
 * <li>
 * tailSequence: this is the side where the youngest item is found. So the tail
 * is the side of the ringbuffer where items are added to.
 * </li>
 * <li>
 * headSequence: this is the side where the oldest items are found. So the head
 * is the side where items gets discarded.
 * </li>
 * </ol>
 * The items in the ringbuffer can be found by a sequence that is in between
 * (inclusive) the head and tail sequence.
 *
 * If data is read from a ringbuffer with a sequence that is smaller than the
 * headSequence, it means that the data is not available anymore and a {@link
 * stale_sequence} is thrown.
 *
 * A Ringbuffer currently is not a distributed data-structure. So all data is
 * stored in a single partition; comparable to the IQueue implementation. But
 * we'll provide an option to partition the data in the near future.
 *
 * A Ringbuffer can be used in a similar way as a queue, but one of the key
 * differences is that a queue.take is destructive, meaning that only 1 thread
 * is able to take an item. A ringbuffer.read is not destructive, so you can
 * have multiple threads reading the same item multiple times.
 *
 * The Ringbuffer is the backing data-structure for the reliable {@link
 * com.hazelcast.core.ITopic} implementation. See
 * {@link com.hazelcast.config.ReliableTopicConfig}.
 *
 */
class HAZELCAST_API ringbuffer : public proxy::RingbufferImpl
{
    friend class spi::ProxyManager;

public:
    /**
     * Adds an item to the tail of the Ringbuffer. If there is no space in the
     * Ringbuffer, the add will overwrite the oldest item in the ringbuffer no
     * matter what the ttl is. For more control on this behavior.
     *
     * The returned value is the sequence of the added item. Using this sequence
     * you can read the added item.
     *
     * <h3>Using the sequence as id</h3>
     * This sequence will always be unique for this Ringbuffer instance so it
     * can be used as a unique id generator if you are publishing items on this
     * Ringbuffer. However you need to take care of correctly determining an
     * initial id when any node uses the ringbuffer for the first time. The most
     * reliable way to do that is to write a dummy item into the ringbuffer and
     * use the returned sequence as initial id. On the reading side, this dummy
     * item should be discard. Please keep in mind that this id is not the
     * sequence of the item you are about to publish but from a previously
     * published item. So it can't be used to find that item.
     *
     * @param item the item to add.
     * @return the sequence of the added item.
     */
    template<typename E>
    boost::future<int64_t> add(const E& item)
    {
        return add_data(to_data(item));
    }

    /**
     * Reads one item from the Ringbuffer.
     *
     * If the sequence is one beyond the current tail, this call blocks until an
     * item is added.
     *
     * This means that the ringbuffer can be processed using the following
     * idiom: <code> Ringbuffer&lt;String&gt; ringbuffer =
     * client.getRingbuffer<E>("rb"); long seq = ringbuffer.headSequence();
     * while(true){
     * String item = ringbuffer.readOne(seq);
     * seq++;
     * ... process item
     * }
     * </code>
     *
     * This method is not destructive unlike e.g. a queue.take. So the same item
     * can be read by multiple readers or it can be read multiple times by the
     * same reader.
     *
     * Currently it isn't possible to control how long this call is going to
     * block. In the future we could add e.g. tryReadOne(long sequence, long
     * timeout, TimeUnit unit).
     *
     * @param sequence the sequence of the item to read.
     * @return the read item
     * @throws stale_sequence             if the sequence is smaller then {@link
     * #headSequence()}. Because a Ringbuffer won't store all event
     * indefinitely, it can be that the data for the given sequence doesn't
     * exist anymore and the {@link stale_sequence} is thrown. It is up to the
     * caller to deal with this particular situation, e.g. throw an Exception or
     * restart from the last known head. That is why the stale_sequence contains
     * the last known head.
     * @throws illegal_argument if sequence is smaller than 0 or larger than
     * {@link #tailSequence()}+1.
     * @throws interrupted               if the call is interrupted while
     * blocking.
     */
    template<typename E>
    boost::future<boost::optional<E>> read_one(int64_t sequence)
    {
        return to_object<E>(read_one_data(sequence));
    }

    /**
     * Asynchronously writes an item with a configurable {@link
     * rb::OverflowPolicy}. <p> If there is space in the Ringbuffer, the call
     * will return the sequence of the written item. If there is no space, it
     * depends on the overflow policy what happens: <ol> <li>{@link
     * rb::OverflowPolicy#OVERWRITE}: we just overwrite the oldest item in the
     * Ringbuffer and we violate the ttl</li> <li>{@link
     * rb::OverflowPolicy#FAIL}: we return -1 </li>
     * </ol>
     * <p>
     * The reason that FAIL exist is to give the opportunity to obey the ttl.
     * If blocking behavior is required, this can be implemented using retrying
     * in combination with an exponential backoff. Example:
     * <pre>{@code
     * int64_t sleepMs = 100;
     * for (; ; ) {
     *   int64_t result = *(ringbuffer.addAsync(item, FAIL)->get());
     *   if (result != -1) {
     *     break;
     *   }
     *   util::sleepMillis(sleepMs);
     *   sleepMs = min(5000, sleepMs * 2);
     * }
     * }</pre>
     * <p>
     *
     * @param item           the item to add
     * @param overflowPolicy the rb::OverflowPolicy to use.
     * @return the sequenceId of the added item, or -1 if the add failed.
     */
    template<typename E>
    boost::future<int64_t> add(const E& item,
                               rb::overflow_policy overflow_policy)
    {
        return addData(to_data(item), overflow_policy);
    }

    /**
     * Adds all the items of a collection to the tail of the Ringbuffer.
     * <p>
     * An add_all is likely to outperform multiple calls to {@link #add(const
     * E&)} due to better io utilization and a reduced number of executed
     * operations. If the batch is empty, the call is ignored. <p> <p> If the
     * collection is larger than the capacity of the Ringbuffer, then the items
     * that were written first will be overwritten. Therefore this call will not
     * block. <p> The items are inserted in the order of the Iterator of the
     * collection. If an add_all is executed concurrently with an add or
     * add_all, no guarantee is given that items are contiguous. <p> The result
     * of the future contains the sequenceId of the last written item. <p>
     *
     * @param collection the batch of items to add.
     * @return the future to synchronize on completion. The result of the future
     * contains the sequenceId of the last written item.
     * @throws illegal_argument if items is empty
     */
    template<typename E>
    boost::future<int64_t> add_all(const std::vector<E>& items,
                                   rb::overflow_policy overflow_policy)
    {
        return add_all_data(to_data_collection(items), overflow_policy);
    }

    /**
     * Reads a batch of items from the Ringbuffer. If the number of available
     * items after the first read item is smaller than the {@code maxCount},
     * these items are returned. So it could be the number of items read is
     * smaller than the {@code maxCount}.
     * <p>
     * If there are less items available than {@code minCount}, then this call
     * blocks.
     * <p>
     * Reading a batch of items is likely to perform better because less
     * overhead is involved.
     * <p>
     * A filter can be provided to only select items that need to be read. If
     * the filter is null, all items are read. If the filter is not null, only
     * items where the filter function returns true are returned. Using filters
     * is a good way to prevent getting items that are of no value to the
     * receiver. This reduces the amount of IO and the number of operations
     * being executed, and can result in a significant performance improvement.
     * <p>
     * For each item not available in the Ringbuffer an attempt is made to read
     * it from the underlying {com.hazelcast.core.RingbufferStore} via
     * multiple invocations of {com.hazelcast.core.RingbufferStore#load(long)},
     * if store is configured for the Ringbuffer. These cases may increase the
     * execution time significantly depending on the implementation of the
     * store. Note that exceptions thrown by the store are propagated to the
     * caller.
     *
     * @param startSequence the startSequence of the first item to read.
     * @param minCount      the minimum number of items to read.
     * @param maxCount      the maximum number of items to read.
     * @param filter        the filter. Filter is allowed to be null, indicating
     *                      there is no filter.
     * @return a future containing the items read.
     * @throws illegal_argument if startSequence is smaller than 0
     *                                  or if startSequence larger than {@link
     * #tailSequence()} or if minCount smaller than 0 or if minCount larger than
     * maxCount, or if maxCount larger than the capacity of the ringbuffer or if
     * maxCount larger than 1000 (to prevent overload)
     */
    template<typename IFUNCTION>
    boost::future<rb::read_result_set> read_many(
      int64_t start_sequence,
      int32_t min_count,
      int32_t max_count,
      const IFUNCTION* filter = nullptr)
    {
        auto filter_data = to_data<IFUNCTION>(filter);
        return read_many_data(
                 start_sequence, min_count, max_count, &filter_data)
          .then(boost::launch::sync,
                [=](boost::future<protocol::ClientMessage> f) {
                    return get_result_set(std::move(f));
                });
    }

    rb::read_result_set get_result_set(boost::future<protocol::ClientMessage> f)
    {
        auto msg = f.get();
        auto* initial_frame =
          reinterpret_cast<protocol::ClientMessage::frame_header_t*>(
            msg.rd_ptr(protocol::ClientMessage::RESPONSE_HEADER_LEN));
        auto read_count = msg.get<int32_t>();
        auto next_seq = msg.get<int64_t>();
        msg.rd_ptr(static_cast<int32_t>(initial_frame->frame_len) -
                   protocol::ClientMessage::RESPONSE_HEADER_LEN -
                   protocol::ClientMessage::INT32_SIZE -
                   protocol::ClientMessage::INT64_SIZE);

        auto datas = msg.get<std::vector<serialization::pimpl::data>>();
        auto item_seqs = msg.get_nullable<std::vector<int64_t>>();
        return rb::read_result_set(read_count,
                                   std::move(datas),
                                   get_serialization_service(),
                                   item_seqs,
                                   next_seq);
    }

    boost::future<rb::read_result_set> read_many(int64_t start_sequence,
                                                 int32_t min_count,
                                                 int32_t max_count)
    {
        return read_many_data(start_sequence, min_count, max_count, nullptr)
          .then(boost::launch::sync,
                [=](boost::future<protocol::ClientMessage> f) {
                    return get_result_set(std::move(f));
                });
    }

private:
    ringbuffer(const std::string& object_name, spi::ClientContext* context)
      : RingbufferImpl(object_name, context)
    {}
};
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
