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
             * @param runnable The runnable to run when this thread is started.
             */
            AbstractThread::AbstractThread(const boost::shared_ptr<Runnable> &runnable, util::ILogger &logger)
                        : state(UNSTARTED), target(runnable), finishedLatch(new util::CountDownLatch(1)), logger(logger) {
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
                if (!state.compareAndSet(UNSTARTED, STARTED)) {
                    return;
                }
                if (target.get() == NULL) {
                    return;
                }

                RunnableInfo *info = new RunnableInfo(target, finishedLatch, logger.shared_from_this());
                startInternal(info);

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
                if (!state.compareAndSet(STARTED, CANCELLED)) {
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

                int64_t threadId = getThreadId();

                wakeup();

                // Note: Do not force cancel since it may cause unreleased lock objects which causes deadlocks.
                // Issue reported at: https://github.com/hazelcast/hazelcast-cpp-client/issues/339

                finishedLatch->await();

                innerJoin();

                startedThreads.remove(threadId);
            }

            bool AbstractThread::join() {
                if (!state.compareAndSet(STARTED, JOINED)) {
                    return false;
                }

                if (isCalledFromSameThread()) {
                    // called from inside the thread, deadlock possibility
                    return false;
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
                                                       const boost::shared_ptr<CountDownLatch> &finishWaitLatch,
                                                       const boost::shared_ptr<ILogger> &logger) : target(target),
                                                                                                   finishWaitLatch(
                                                                                                           finishWaitLatch),
                                                                                                   logger(logger) {}
        }
    }
}
