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

#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {
        
        namespace test {
            class FailFastCallIdSequenceTest : public ClientTestSupport {
            public:
            };

            TEST_F(FailFastCallIdSequenceTest, testGettersAndDefaults) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                ASSERT_EQ(0, sequence.getLastCallId());
                ASSERT_EQ(100, sequence.getMaxConcurrentInvocations());
            }

            TEST_F(FailFastCallIdSequenceTest, whenNext_thenSequenceIncrements) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                int64_t oldSequence = sequence.getLastCallId();
                int64_t result = sequence.next();
                ASSERT_EQ(oldSequence + 1, result);
                ASSERT_EQ(oldSequence + 1, sequence.getLastCallId());
            }

            TEST_F(FailFastCallIdSequenceTest, next_whenNoCapacity_thenThrowException) {
                spi::impl::sequence::FailFastCallIdSequence sequence(1);

                    // take the only slot available
                    sequence.next();

                    // this next is going to fail with an exception
                    ASSERT_THROW(sequence.next(), exception::HazelcastOverloadException);
            }

            TEST_F(FailFastCallIdSequenceTest, when_overCapacityButPriorityItem_then_noException) {
                spi::impl::sequence::FailFastCallIdSequence sequence(1);

                // take the only slot available
                ASSERT_EQ(1, sequence.next());

                ASSERT_EQ(2, sequence.forceNext());
            }

            TEST_F(FailFastCallIdSequenceTest, whenComplete_thenTailIncrements) {
                spi::impl::sequence::FailFastCallIdSequence sequence(100);
                sequence.next();

                int64_t oldSequence = sequence.getLastCallId();
                int64_t oldTail = sequence.getTail();
                sequence.complete();

                ASSERT_EQ(oldSequence, sequence.getLastCallId());
                ASSERT_EQ(oldTail + 1, sequence.getTail());
            }

        }
    }
}
