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
#ifndef HAZELCAST_UTIL_WINDOWSTHREAD_INL_
#define HAZELCAST_UTIL_WINDOWSTHREAD_INL_

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "hazelcast/util/impl/AbstractThread.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {

        class HAZELCAST_API Thread : public impl::AbstractThread {
        public:
            Thread(const std::string &name) : impl::AbstractThread(name) {
            }

            virtual ~Thread() {
                if (started) {
                    cancel();
                    join();
                }

                CloseHandle(thread);
            }

            void interruptibleSleep(int seconds) {
                LockGuard lock(wakeupMutex);
                if(isInterrupted){
                    isInterrupted = false;
                    throw hazelcast::client::exception::InterruptedException("interruptibleSleep");
                }
                bool wokenUpbyInterruption = wakeupCondition.waitFor(wakeupMutex, seconds * 1000);
                if(wokenUpbyInterruption && isInterrupted){
                    isInterrupted = false;
                    throw hazelcast::client::exception::InterruptedException("interruptibleSleep");
                }
            }

            void wakeup() {
                LockGuard guard(wakeupMutex);
                wakeupCondition.notify();
            }

            void cancel() {
                LockGuard lock(wakeupMutex);
                isInterrupted = true;
                wakeupCondition.notify_all();
            }

            bool join() {
                if (!isJoined.compareAndSet(false, true)) {
                    return true;
                }
                if (id == getThreadID()) {
                    // called from inside the thread, deadlock possibility
                    return false;
                }

                DWORD err = WaitForSingleObject(thread, INFINITE);
                if (err != WAIT_OBJECT_0) {
                    return false;
                }
                isJoined = true;
                return true;
            }

            void startInternal(Runnable *targetObject) {
                thread = CreateThread(NULL, 0, impl::AbstractThread::runnableThread, runnableObject, 0 , &id);
            }

            virtual long getThreadID() {
                return GetCurrentThreadId();
            }

        protected:
            util::AtomicBoolean isInterrupted;
            HANDLE thread;
            DWORD id;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_WINDOWSTHREAD_INL_


