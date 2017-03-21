/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_FUTURE_H_
#define HAZELCAST_CLIENT_FUTURE_H_

#include <stdint.h>
#include <boost/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        template <typename V>
        class Future {
        public:
            /**
             * Waits if necessary for the computation to complete, and then
             * retrieves its result.
             *
             * @return the computed result
             * @throws ExecutionException if the computation threw an
             * exception
             * @throws InterruptedException if the current thread was interrupted
             * while waiting
             */
            virtual boost::shared_ptr<V> get() = 0;

            /**
             * Waits if necessary for at most the given time for the computation
             * to complete, and then retrieves its result, if available.
             *
             * @param timeoutInMilliseconds the maximum time to wait in milliseconds
             * @return the computed result
             * @throws exception::TimeoutException if the wait timed out
             */
            virtual boost::shared_ptr<V> get(int64_t timeoutInMilliseconds) = 0;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_FUTURE_H_ */
