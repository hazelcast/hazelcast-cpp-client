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

#include <cerrno>
#include "hazelcast/util/impl/AbstractThread.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace util {
        namespace impl {
            util::SynchronizedMap<long, AbstractThread::UnmanagedAbstractThreadPointer> AbstractThread::startedThreads;

            AbstractThread::AbstractThread(const boost::shared_ptr<Runnable> &runnable) : target(runnable) {
                this->target = runnable;
            }

            AbstractThread::~AbstractThread() {
            }

            const std::string AbstractThread::getName() const {
                if (target.get() == NULL) {
                    return "";
                }

                return target->getName();
            }

            void AbstractThread::start() {
                if (!started.compareAndSet(false, true)) {
                    return;
                }
                if (target.get() == NULL) {
                    return;
                }

                startInternal(target.get());

                startedThreads.put(getThreadId(), boost::shared_ptr<UnmanagedAbstractThreadPointer>(
                        new UnmanagedAbstractThreadPointer(this)));
            }

            void AbstractThread::interruptibleSleep(int seconds) {
                interruptibleSleepMillis(seconds * 1000);
            }

            void AbstractThread::interruptibleSleepMillis(int64_t timeInMillis) {
                LockGuard guard(wakeupMutex);
                wakeupCondition.waitFor(wakeupMutex, timeInMillis);
            }

            void AbstractThread::wakeup() {
                LockGuard guard(wakeupMutex);
                wakeupCondition.notify();
            }

            void AbstractThread::cancel() {
                if (!started) {
                    return;
                }

                if (isCalledFromSameThread()) {
                    /**
                     * do not allow cancelling itself
                     * at Linux, pthread_cancel may cause cancel by signal
                     * and calling thread may be terminated.
                     */
                    return;
                }

                if (!isJoined) {
                    wakeup();

                    // Note: Do not force cancel since it may cause unreleased lock objects which causes deadlocks.
                    // Issue reported at: https://github.com/hazelcast/hazelcast-cpp-client/issues/339
                }
            }

            bool AbstractThread::join() {
                if (!started) {
                    return false;
                }

                if (isCalledFromSameThread()) {
                    // called from inside the thread, deadlock possibility
                    return false;
                }

                if (!isJoined.compareAndSet(false, true)) {
                    return true;
                }

                long threadId = getThreadId();

                if (!innerJoin()) {
                    return false;
                }

                startedThreads.remove(threadId);
                isJoined = true;
                return true;
            }

            void AbstractThread::sleep(int64_t timeInMilliseconds) {
                long currentThreadId = util::getThreadId();
                boost::shared_ptr<UnmanagedAbstractThreadPointer> currentThread = startedThreads.get(currentThreadId);
                // this method should only be called from a started thread!!!
                assert(currentThread.get() != NULL);
                if (currentThread.get()) {
                    currentThread->getThread()->interruptibleSleepMillis(timeInMilliseconds);
                }
            }

            AbstractThread::UnmanagedAbstractThreadPointer::UnmanagedAbstractThreadPointer(AbstractThread *thread)
                    : thread(thread) {}

            AbstractThread *AbstractThread::UnmanagedAbstractThreadPointer::getThread() const {
                return thread;
            }
        }
    }
}
