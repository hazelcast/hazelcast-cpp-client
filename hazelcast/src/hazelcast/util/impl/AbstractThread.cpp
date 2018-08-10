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
#include <hazelcast/util/ILogger.h>
#include "hazelcast/util/impl/AbstractThread.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace util {
        namespace impl {
            util::SynchronizedMap<int64_t, AbstractThread::UnmanagedAbstractThreadPointer> AbstractThread::startedThreads;

            /**
             * Latch count is 2, because one is count down when thread started and the second one is decremented when
             * thread exits. Derived classes should decrement this latch just before exiting the thread
             * execution method.
             * @param runnable The runnable to run when this thread is started.
             */
            AbstractThread::AbstractThread(const boost::shared_ptr<Runnable> &runnable) : target(runnable), stateLatch(
                    new util::CountDownLatch(2)) {
                this->target = runnable;
            }

            AbstractThread::~AbstractThread() {
                if (started) {
                    int state = stateLatch->get();
                    if (state == 1) {
                        stateLatch->await();
                    }
                }
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

                RunnableInfo *info = new RunnableInfo(target, stateLatch);
                startInternal(info);
                stateLatch->countDown();

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

                int64_t threadId = getThreadId();

                if (!innerJoin()) {
                    return false;
                }

                startedThreads.remove(threadId);
                return true;
            }

            void AbstractThread::sleep(int64_t timeInMilliseconds) {
                int64_t currentThreadId = util::getCurrentThreadId();
                boost::shared_ptr<UnmanagedAbstractThreadPointer> currentThread = startedThreads.get(currentThreadId);
                if (currentThread.get()) {
                    currentThread->getThread()->interruptibleSleepMillis(timeInMilliseconds);
                } else {
                    util::ILogger::getLogger().warning()
                            << "AbstractThread::sleep IS NOT interrupptible SLEEP!!! currentThreadId:"
                            << currentThreadId;
                    util::sleepmillis(timeInMilliseconds);
                }
            }

            const boost::shared_ptr<Runnable> &AbstractThread::getTarget() const {
                return target;
            }

            AbstractThread::UnmanagedAbstractThreadPointer::UnmanagedAbstractThreadPointer(AbstractThread *thread)
                    : thread(thread) {}

            AbstractThread *AbstractThread::UnmanagedAbstractThreadPointer::getThread() const {
                return thread;
            }

            AbstractThread::RunnableInfo::RunnableInfo(const boost::shared_ptr<Runnable> &target,
                                                       const boost::shared_ptr<CountDownLatch> &latch) : target(target),
                                                                                                         latch(latch) {}
        }
    }
}
