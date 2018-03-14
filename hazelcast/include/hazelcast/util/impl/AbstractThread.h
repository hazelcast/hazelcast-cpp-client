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

namespace hazelcast {
    namespace util {
        namespace impl {
            class HAZELCAST_API AbstractThread : public Runnable {
            public:
                AbstractThread(const std::string &name);

                AbstractThread(const boost::shared_ptr<Runnable> &runnable);

                virtual ~AbstractThread();

                virtual const std::string getName() const;

                virtual void interruptibleSleep(int seconds) = 0;

                virtual void wakeup() = 0;

                virtual void cancel() = 0;

                virtual bool join() = 0;

                void start();

                virtual long getThreadID() = 0;

            protected:
                static void *runnableThread(void *args);

                virtual void startInternal(Runnable *targetObject) = 0;

                std::string threadName;
                util::AtomicBoolean isJoined;
                util::AtomicBoolean started;
                ConditionVariable wakeupCondition;
                Mutex wakeupMutex;
                boost::shared_ptr<Runnable> target;
            };
        }
    }
}

#endif //HAZELCAST_UTIL_IMPL_ABSTRACTTHREAD_H_


