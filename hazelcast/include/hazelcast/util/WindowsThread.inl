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

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "hazelcast/util/impl/AbstractThread.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {

        class HAZELCAST_API Thread : public impl::AbstractThread {
        public:
            Thread(const boost::shared_ptr<Runnable> &runnable, util::ILogger &logger)
                : impl::AbstractThread(runnable, logger) {
            }

            virtual ~Thread() {
                cancel();

                if (handleClosed.compareAndSet(false, true)) {
                    CloseHandle(thread);
                }
            }

            virtual int64_t getThreadId() {
                return (int64_t) id;
            }

            static void yield() {
                SwitchToThread();
            }

        protected:
            static DWORD WINAPI runnableThread(LPVOID args) {
                RunnableInfo *info = static_cast<RunnableInfo *>(args);

                boost::shared_ptr<Runnable> target = info->target;
                try {
                    target->run();
                } catch (hazelcast::client::exception::InterruptedException &e) {
                    info->logger->warning() << "Thread " << target->getName() << " is interrupted. " << e;
                } catch (hazelcast::client::exception::IException &e) {
                    info->logger->warning() << "Thread " << target->getName() << " is cancelled with exception " << e;
                } catch (...) {
                    info->logger->warning() << "Thread " << target->getName() << " is cancelled with an unexpected exception";

                    info->finishWaitLatch->countDown();

                    delete info;

                    return 1L;
                }

                info->logger->finest() << "Thread " << target->getName() << " is finished.";

                info->finishWaitLatch->countDown();

                delete info;

                return 0;
            }

            void startInternal(RunnableInfo *info) {
                thread = CreateThread(NULL, 0, runnableThread, info, 0 , &id);
            }

            virtual bool isCalledFromSameThread() {
                return id == util::getCurrentThreadId();
            }

            virtual bool innerJoin() {
                DWORD err = WaitForSingleObject(thread, INFINITE);
                if (err != WAIT_OBJECT_0) {
                    return false;
                }
                return true;
            }

            HANDLE thread;
            DWORD id;
            util::AtomicBoolean handleClosed;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_WINDOWSTHREAD_INL_


