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
#ifndef HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_
#define HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_

#include <cstdlib>

#include <boost/shared_ptr.hpp>

#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/SynchronizedMap.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        namespace impl {
            class HAZELCAST_API AbstractThread {
            public:
                class UnmanagedAbstractThreadPointer {
                public:
                    UnmanagedAbstractThreadPointer(AbstractThread *thread);

                    AbstractThread *getThread() const;

                private:
                    AbstractThread *thread;
                };

                AbstractThread(const boost::shared_ptr<Runnable> &runnable);

                virtual ~AbstractThread();

                virtual const std::string getName() const;

                virtual void interruptibleSleep(int seconds);

                void interruptibleSleepMillis(int64_t timeInMillis);

                static void sleep(int64_t timeInMilliseconds);

                virtual void wakeup();

                virtual void cancel();

                virtual bool join();

                void start();

                virtual long getThreadId() = 0;

            protected:
                virtual void startInternal(Runnable *targetObject) = 0;
                virtual bool isCalledFromSameThread() = 0;
                virtual bool innerJoin() = 0;

                util::AtomicBoolean isJoined;
                util::AtomicBoolean started;
                ConditionVariable wakeupCondition;
                static util::SynchronizedMap<long, UnmanagedAbstractThreadPointer> startedThreads;

                Mutex wakeupMutex;

                boost::shared_ptr<Runnable> target;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_


