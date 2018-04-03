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

#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace sequence {

                    std::auto_ptr<CallIdSequence> CallIdFactory::newCallIdSequence(bool isBackPressureEnabled,
                                                                                   int32_t maxAllowedConcurrentInvocations,
                                                                                   int64_t backoffTimeoutMs) {
                        if (!isBackPressureEnabled) {
                            return std::auto_ptr<CallIdSequence>(new CallIdSequenceWithoutBackpressure());
                        } else if (backoffTimeoutMs <= 0) {
                            return std::auto_ptr<CallIdSequence>(
                                    new FailFastCallIdSequence(maxAllowedConcurrentInvocations));
                        } else {
                            return std::auto_ptr<CallIdSequence>(
                                    new CallIdSequenceWithBackpressure(maxAllowedConcurrentInvocations,
                                                                       backoffTimeoutMs));
                        }
                    }
                }
            }
        }

    }
}
