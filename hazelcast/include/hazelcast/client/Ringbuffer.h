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
// Created by ihsan demir on 16/20/13.
#ifndef HZELCAST_CLIENT_RINGBUFFER_H_
#define HZELCAST_CLIENT_RINGBUFFER_H_

#include <memory>
#include <stdint.h>

#include "hazelcast/client/IDistributedObject.h"

namespace hazelcast {
    namespace client {
        /**
         * A Ringbuffer is a data-structure where the content is stored in a ring like structure. A ringbuffer has a capacity so it
         * won't grow beyond that capacity and endanger the stability of the system. If that capacity is exceeded, than the oldest
         * item in the ringbuffer is overwritten.
         *
         * The ringbuffer has 2 always incrementing sequences:
         * <ol>
         * <li>
         * tailSequence: this is the side where the youngest item is found. So the tail is the side of the ringbuffer where
         * items are added to.
         * </li>
         * <li>
         * headSequence: this is the side where the oldest items are found. So the head is the side where items gets
         * discarded.
         * </li>
         * </ol>
         * The items in the ringbuffer can be found by a sequence that is in between (inclusive) the head and tail sequence.
         *
         * If data is read from a ringbuffer with a sequence that is smaller than the headSequence, it means that the data
         * is not available anymore and a {@link StaleSequenceException} is thrown.
         *
         * A Ringbuffer currently is not a distributed data-structure. So all data is stored in a single partition; comparable to the
         * IQueue implementation. But we'll provide an option to partition the data in the near future.
         *
         * A Ringbuffer can be used in a similar way as a queue, but one of the key differences is that a queue.take is destructive,
         * meaning that only 1 thread is able to take an item. A ringbuffer.read is not destructive, so you can have multiple threads
         * reading the same item multiple times.
         *
         * The Ringbuffer is the backing data-structure for the reliable {@link com.hazelcast.core.ITopic} implementation. See
         * {@link com.hazelcast.config.ReliableTopicConfig}.
         *
         * @param <E>
         */
        template<typename E>
        class Ringbuffer : public IDistributedObject {
        public:
            virtual ~Ringbuffer() {
            }

            /**
             * Returns the capacity of this Ringbuffer.
             *
             * @return the capacity.
             */
            virtual int64_t capacity() = 0;

            /**
             * Returns number of items in the ringbuffer.
             *
             * If no ttl is set, the size will always be equal to capacity after the head completed the first loop
             * around the ring. This is because no items are getting retired.
             *
             * @return the size.
             */
            virtual int64_t size() = 0;

            /**
             * Returns the sequence of the tail. The tail is the side of the ringbuffer where the items are added to.
             *
             * The initial value of the tail is -1.
             *
             * @return the sequence of the tail.
             */
            virtual int64_t tailSequence() = 0;

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
            virtual int64_t headSequence() = 0;

            /**
             * Returns the remaining capacity of the ringbuffer.
             *
             * The returned value could be stale as soon as it is returned.
             *
             * If ttl is not set, the remaining capacity will always be the capacity.
             *
             * @return the remaining capacity.
             */
            virtual int64_t remainingCapacity() = 0;

            /**
             * Adds an item to the tail of the Ringbuffer. If there is no space in the Ringbuffer, the add will overwrite the oldest
             * item in the ringbuffer no matter what the ttl is. For more control on this behavior.
             *
             * The returned value is the sequence of the added item. Using this sequence you can read the added item.
             *
             * <h3>Using the sequence as id</h3>
             * This sequence will always be unique for this Ringbuffer instance so it can be used as a unique id generator if you are
             * publishing items on this Ringbuffer. However you need to take care of correctly determining an initial id when any node
             * uses the ringbuffer for the first time. The most reliable way to do that is to write a dummy item into the ringbuffer and
             * use the returned sequence as initial id. On the reading side, this dummy item should be discard. Please keep in mind that
             * this id is not the sequence of the item you are about to publish but from a previously published item. So it can't be used
             * to find that item.
             *
             * @param item the item to add.
             * @return the sequence of the added item.
             */
            virtual int64_t add(const E &item) = 0;

            /**
             * Reads one item from the Ringbuffer.
             *
             * If the sequence is one beyond the current tail, this call blocks until an item is added.
             *
             * This means that the ringbuffer can be processed using the following idiom:
             * <code>
             * Ringbuffer&lt;String&gt; ringbuffer = client.getRingbuffer<E>("rb");
             * long seq = ringbuffer.headSequence();
             * while(true){
             * String item = ringbuffer.readOne(seq);
             * seq++;
             * ... process item
             * }
             * </code>
             *
             * This method is not destructive unlike e.g. a queue.take. So the same item can be read by multiple readers or it can be
             * read multiple times by the same reader.
             *
             * Currently it isn't possible to control how long this call is going to block. In the future we could add e.g.
             * tryReadOne(long sequence, long timeout, TimeUnit unit).
             *
             * @param sequence the sequence of the item to read.
             * @return the read item
             * @throws StaleSequenceException             if the sequence is smaller then {@link #headSequence()}. Because a
             *                                            Ringbuffer won't store all event indefinitely, it can be that the data for the
             *                                            given sequence doesn't exist anymore and the {@link StaleSequenceException}
             *                                            is thrown. It is up to the caller to deal with this particular situation, e.g.
             *                                            throw an Exception or restart from the last known head. That is why the
             *                                            StaleSequenceException contains the last known head.
             * @throws java.lang.IllegalArgumentException if sequence is smaller than 0 or larger than {@link #tailSequence()}+1.
             * @throws InterruptedException               if the call is interrupted while blocking.
             */
            virtual std::auto_ptr<E> readOne(int64_t sequence) = 0;
        };
    }
}

#endif //HZELCAST_CLIENT_RINGBUFFER_H_

