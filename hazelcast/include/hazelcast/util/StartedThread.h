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
#ifndef HAZELCAST_UTIL_STARTED_THREAD_H_
#define HAZELCAST_UTIL_STARTED_THREAD_H_

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/ThreadArgs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class HAZELCAST_API RunnableDelegator : public Runnable {
        public:
            RunnableDelegator(Runnable &runnable);

            virtual void run();

            virtual const std::string getName() const;

        private:
            Runnable &runnable;
        };

        class HAZELCAST_API StartedThread : public Thread {
        public:
            StartedThread(const std::string &name, void (*func)(ThreadArgs &),
                          void *arg0 = NULL, void *arg1 = NULL, void *arg2 = NULL, void *arg3 = NULL);

            StartedThread(void (func)(ThreadArgs &),
                          void *arg0 = NULL,
                          void *arg1 = NULL,
                          void *arg2 = NULL,
                          void *arg3 = NULL);

            virtual void run();

        private:
            ThreadArgs threadArgs;

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3 );
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_STARTED_THREAD_H_


