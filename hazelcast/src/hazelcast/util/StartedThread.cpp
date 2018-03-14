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

#include "hazelcast/util/StartedThread.h"

namespace hazelcast {
    namespace util {
        StartedThread::StartedThread(const std::string &name, void (*func)(ThreadArgs &),
                                     void *arg0, void *arg1, void *arg2, void *arg3)
                : Thread(name) {
            init(func, arg0, arg1, arg2, arg3);
        }

        StartedThread::StartedThread(void (func)(ThreadArgs &),
                                     void *arg0,
                                     void *arg1,
                                     void *arg2,
                                     void *arg3)
                : Thread("hz.unnamed") {
            init(func, arg0, arg1, arg2, arg3);
        }

        void StartedThread::init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3) {
            threadArgs.arg0 = arg0;
            threadArgs.arg1 = arg1;
            threadArgs.arg2 = arg2;
            threadArgs.arg3 = arg3;
            threadArgs.func = func;
            threadArgs.currentThread = this;
            start();
        }

        void StartedThread::run() {
            threadArgs.func(threadArgs);
        }

        RunnableDelegator::RunnableDelegator(Runnable & runnable) : runnable(runnable) {}

        void RunnableDelegator::run() {
            runnable.run();
        }

        const std::string RunnableDelegator::getName() const {
            return runnable.getName();
        }

    }
}
