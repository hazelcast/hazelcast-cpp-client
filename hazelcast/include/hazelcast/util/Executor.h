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

#ifndef HAZELCAST_UTIL_EXECUTOR_H_
#define HAZELCAST_UTIL_EXECUTOR_H_

#include <boost/shared_ptr.hpp>

#include "hazelcast/util/Runnable.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API Executor {
        public:
            virtual void execute(const boost::shared_ptr<Runnable> &command) = 0;
        };

        class HAZELCAST_API ExecutorService : public Executor {
        public:
            /**
             * Initiates an orderly shutdown in which previously submitted
             * tasks are executed, but no new tasks will be accepted.
             * Invocation has no additional effect if already shut down.
             *
             * <p>This method does not wait for previously submitted tasks to
             * complete execution.  Use {@link #awaitTermination awaitTermination}
             * to do that.
             *
             */
            virtual void shutdown() = 0;
        };
    }
}


#endif //HAZELCAST_UTIL_EXECUTOR_H_
