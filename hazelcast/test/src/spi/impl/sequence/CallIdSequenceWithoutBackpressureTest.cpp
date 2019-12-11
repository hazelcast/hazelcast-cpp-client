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

#include "ClientTestSupport.h"

#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/util/CountDownLatch.h>
#include <hazelcast/util/Thread.h>

namespace hazelcast {
    namespace client {

        namespace test {
            class CallIdSequenceWithoutBackpressureTest : public ClientTestSupport {
            protected:
                spi::impl::sequence::CallIdSequenceWithoutBackpressure sequence;

                void next(bool isUrgent) {
                    int64_t oldSequence = sequence.getLastCallId();
                    int64_t result = nextCallId(sequence, isUrgent);
                    assertEquals(oldSequence + 1, result);
                    assertEquals(oldSequence + 1, sequence.getLastCallId());
                }
                
                int64_t nextCallId(spi::impl::sequence::CallIdSequence &seq, bool isUrgent) {
                    return isUrgent ? seq.forceNext() : seq.next();
                }
            };

            TEST_F(CallIdSequenceWithoutBackpressureTest, testInit) {
                assertEquals(0, sequence.getLastCallId());
                assertEquals(INT32_MAX, sequence.getMaxConcurrentInvocations());
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, testNext) {
                // regular operation
                next(false);
                next(true);
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, whenNextRepeated_thenKeepSucceeding) {
                for (int64_t k = 1; k < 10000; k++) {
                    assertEquals(k, nextCallId(sequence, false));
                }
            }

            TEST_F(CallIdSequenceWithoutBackpressureTest, complete) {
                nextCallId(sequence, false);
                int64_t oldSequence = sequence.getLastCallId();
                sequence.complete();
                assertEquals(oldSequence, sequence.getLastCallId());
            }
        }
    }
}
