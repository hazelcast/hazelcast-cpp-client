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
#ifndef HAZELCAST_UTIL_POSIXTHREAD_INL_
#define HAZELCAST_UTIL_POSIXTHREAD_INL_

#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "hazelcast/util/impl/AbstractThread.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace util {
        class Thread : public impl::AbstractThread {
        public:
            Thread(const boost::shared_ptr<Runnable> &runnable) : impl::AbstractThread(runnable) {
                initAttributes();
            }

            virtual ~Thread() {
                if (started) {
                    cancel();
                    join();
                }

                pthread_attr_destroy(&attr);
            }

            virtual int64_t getThreadId() {
                int64_t threadId = 0;
                memcpy(&threadId, &thread, std::min(sizeof(threadId), sizeof(thread)));
                return threadId;
            }

            static void yield() {
                #ifdef __linux__
                pthread_yield();
                #else
                pthread_yield_np();
                #endif
            }

        protected:
            void initAttributes() {
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            }

            static void *runnableThread(void *args) {
                Runnable *runnable = static_cast<Runnable *>(args);
                ILogger &logger = ILogger::getLogger();
                try {
                    runnable->run();
                } catch (hazelcast::client::exception::InterruptedException &e) {
                    logger.finest() << "Thread " << runnable->getName() << " is interrupted. " << e;
                } catch (hazelcast::client::exception::IException &e) {
                    logger.warning() << "Thread " << runnable->getName() << " is cancelled with exception " << e;
                } catch (...) {
                    logger.warning() << "Thread " << runnable->getName()
                                     << " is cancelled with an unexpected exception";
                    throw;
                }

                logger.finest() << "Thread " << runnable->getName() << " is finished.";

                return NULL;
            }

            void startInternal(Runnable *targetObject) {
                pthread_create(&thread, &attr, runnableThread, targetObject);
                started = true;
            }

            virtual bool isCalledFromSameThread() {
                return pthread_equal(thread, pthread_self()) != 0;
            }

            virtual bool innerJoin() {
                int err = pthread_join(thread, NULL);
                if (EINVAL == err || ESRCH == err || EDEADLK == err) {
                    isJoined = false;
                    return false;
                }
                return true;
            }

            pthread_t thread;
            pthread_attr_t attr;
        };
    }
}

#endif //HAZELCAST_UTIL_POSIXTHREAD_INL_


