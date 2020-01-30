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
#ifndef HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_
#define HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_

#include <cstdlib>

#include <memory>

#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/CountDownLatch.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;

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

                AbstractThread(const std::shared_ptr<Runnable> &runnable, util::ILogger &logger);

                virtual ~AbstractThread();

                virtual const std::string getName() const;

                virtual void interruptibleSleep(int seconds);

                void interruptibleSleepMillis(int64_t timeInMillis);

                static void sleep(int64_t timeInMilliseconds);

                virtual void wakeup();

                virtual void cancel();

                virtual bool join();

                void start();

                virtual int64_t getThreadId() = 0;

                const std::shared_ptr<Runnable> &getTarget() const;

                bool waitMilliseconds(int64_t milliseconds);

            protected:
                enum ThreadState {
                    UNSTARTED = 0,
                    STARTED = 1,
                    JOINED = 2,
                    CANCELLED = 3
                };

                struct RunnableInfo {
                    RunnableInfo(const std::shared_ptr<Runnable> &target,
                                 const std::shared_ptr<CountDownLatch> &finishWaitLatch,
                                 const std::shared_ptr<ILogger> &logger);

                    std::shared_ptr<Runnable> target;
                    std::shared_ptr<util::CountDownLatch> finishWaitLatch;
                    std::shared_ptr<util::ILogger> logger;
                };

                virtual void startInternal(RunnableInfo *info) = 0;
                virtual bool isCalledFromSameThread() = 0;
                virtual bool innerJoin() = 0;

                util::AtomicBoolean isJoined;
                std::atomic<ThreadState> state;
                util::AtomicBoolean started;
                util::AtomicBoolean cancelled;
                ConditionVariable wakeupCondition;
                static util::SynchronizedMap<int64_t, UnmanagedAbstractThreadPointer> startedThreads;
                Mutex wakeupMutex;
                std::shared_ptr<Runnable> target;
                std::shared_ptr<util::CountDownLatch> finishedLatch;
                util::ILogger &logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_


