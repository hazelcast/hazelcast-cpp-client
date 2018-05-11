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
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/TimeUtil.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {
                    const std::auto_ptr<util::concurrent::IdleStrategy> CallIdSequenceWithBackpressure::IDLER(
                            new util::concurrent::BackoffIdleStrategy(
                                    0, 0, boost::posix_time::microseconds(1000).total_nanoseconds(),
                                    boost::posix_time::microseconds(MAX_DELAY_MS * 1000).total_nanoseconds()));

                    CallIdSequenceWithBackpressure::CallIdSequenceWithBackpressure(int32_t maxConcurrentInvocations,
                                                                                   int64_t backoffTimeoutMs)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {
                        std::ostringstream out;
                        out << "backoffTimeoutMs should be a positive number. backoffTimeoutMs=" << backoffTimeoutMs;
                        util::Preconditions::checkPositive(backoffTimeoutMs, out.str());

                        backoffTimeoutNanos = boost::posix_time::milliseconds(backoffTimeoutMs).total_nanoseconds();
                    }

                    void CallIdSequenceWithBackpressure::handleNoSpaceLeft() {
                        boost::posix_time::time_duration start = util::TimeUtil::getDurationSinceEpoch();
                        for (int64_t idleCount = 0;; idleCount++) {
                            int64_t elapsedNanos = (util::TimeUtil::getDurationSinceEpoch() - start).total_nanoseconds();
                            if (elapsedNanos > backoffTimeoutNanos) {
                                throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                        "CallIdSequenceWithBackpressure::handleNoSpaceLeft")
                                        << "Timed out trying to acquire another call ID."
                                        << " maxConcurrentInvocations = " << getMaxConcurrentInvocations()
                                        << ", backoffTimeout = " << boost::posix_time::microseconds(
                                        backoffTimeoutNanos / 1000).total_milliseconds() << " msecs, elapsed:"
                                        << boost::posix_time::microseconds(elapsedNanos / 1000).total_milliseconds()
                                        << " msecs").build();
                            }
                            IDLER->idle(idleCount);
                            if (hasSpace()) {
                                return;
                            }

                        }
                    }
                }
            }

        }
    }
}
