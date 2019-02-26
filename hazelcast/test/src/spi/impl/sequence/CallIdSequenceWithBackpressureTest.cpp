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

#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/util/CountDownLatch.h>
#include <hazelcast/util/Thread.h>
#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {

        namespace test {
            class CallIdSequenceWithBackpressureTest : public ClientTestSupport {
            public:
                CallIdSequenceWithBackpressureTest() {}

            protected:
                class ThreeSecondDelayCompleteOperation : public util::Runnable {
                public:
                    ThreeSecondDelayCompleteOperation(spi::impl::sequence::CallIdSequenceWithBackpressure &sequence,
                                                      util::CountDownLatch &nextCalledLatch) : sequence(sequence),
                                                                                               nextCalledLatch(
                                                                                                       nextCalledLatch) {}

                private:
                    virtual const std::string getName() const {
                        return "ThreeSecondDelayCompleteOperation";
                    }

                    virtual void run() {
                        sequence.next();
                        nextCalledLatch.countDown();
                        sleepSeconds(3);
                        sequence.complete();
                    }

                private:
                    spi::impl::sequence::CallIdSequenceWithBackpressure &sequence;
                    util::CountDownLatch &nextCalledLatch;
                };

                int64_t nextCallId(spi::impl::sequence::CallIdSequence &seq, bool isUrgent) {
                    return isUrgent ? seq.forceNext() : seq.next();
                }
            };

            TEST_F(CallIdSequenceWithBackpressureTest, testInit) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(100, 60000);
                assertEquals(0, sequence.getLastCallId());
                assertEquals(100, sequence.getMaxConcurrentInvocations());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, whenNext_thenSequenceIncrements) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(100, 60000);
                int64_t oldSequence = sequence.getLastCallId();
                int64_t result = sequence.next();
                assertEquals(oldSequence + 1, result);
                assertEquals(oldSequence + 1, sequence.getLastCallId());

                oldSequence = sequence.getLastCallId();
                result = sequence.forceNext();
                assertEquals(oldSequence + 1, result);
                assertEquals(oldSequence + 1, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, next_whenNoCapacity_thenBlockTillCapacity) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);
                int64_t oldLastCallId = sequence.getLastCallId();

                util::CountDownLatch nextCalledLatch(1);

                util::Thread t(boost::shared_ptr<util::Runnable>(
                        new ThreeSecondDelayCompleteOperation(sequence, nextCalledLatch)), getLogger());
                t.start();

                ASSERT_OPEN_EVENTUALLY(nextCalledLatch);

                int64_t result = sequence.next();
                assertEquals(oldLastCallId + 2, result);
                assertEquals(oldLastCallId + 2, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, next_whenNoCapacity_thenBlockTillTimeout) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 2000);
                // first invocation consumes the available call ID
                nextCallId(sequence, false);

                int64_t oldLastCallId = sequence.getLastCallId();
                ASSERT_THROW(sequence.next(), exception::HazelcastOverloadException);

                assertEquals(oldLastCallId, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, when_overCapacityButPriorityItem_then_noBackpressure) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);

                // occupy the single call ID slot
                nextCallId(sequence, true);

                int64_t oldLastCallId = sequence.getLastCallId();

                int64_t result = nextCallId(sequence, true);
                assertEquals(oldLastCallId + 1, result);
                assertEquals(oldLastCallId + 1, sequence.getLastCallId());
            }

            TEST_F(CallIdSequenceWithBackpressureTest, whenComplete_thenTailIncrements) {
                spi::impl::sequence::CallIdSequenceWithBackpressure sequence(1, 60000);

                nextCallId(sequence, false);

                int64_t oldSequence = sequence.getLastCallId();
                int64_t oldTail = sequence.getTail();
                sequence.complete();

                assertEquals(oldSequence, sequence.getLastCallId());
                assertEquals(oldTail + 1, sequence.getTail());
            }

        }
    }
}
