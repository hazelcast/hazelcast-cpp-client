/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Mutex.h"

#include <memory>
#include <cassert>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class Future {
        public:
            Future()
            : resultReady(false)
            , exceptionReady(false) {

            }

            void set_value(T& value) {
                LockGuard guard(mutex);
                if (exceptionReady || resultReady) {
                    util::ILogger::getLogger().warning(std::string("Future.set_value should not be called twice"));
                }
                sharedObject = value;
                resultReady = true;
                conditionVariable.notify_all();
            }

            void set_exception(std::auto_ptr<client::exception::IException> exception) {
                LockGuard guard(mutex);

                if (exceptionReady || resultReady) {
                    util::ILogger::getLogger().warning(std::string("Future.set_exception should not be called twice : details ") + exception->what());
                }

                this->exception = exception;
                exceptionReady = true;
                conditionVariable.notify_all();
            }

            void reset_exception(std::auto_ptr<client::exception::IException> exception) {
                LockGuard guard(mutex);

                this->exception = exception;
                exceptionReady = true;
                conditionVariable.notify_all();
            }

            T get() {
                LockGuard guard(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    exception->raise();
                }
                assert(false && "InvalidState");
                return sharedObject;
            }

            /**
             *
             * @return true if result or exception is ready. false otherwise when timeout expires.
             *
             * Does not throw
             */
            bool waitFor(int64_t timeInMilliseconds) {
                int64_t endTime = util::currentTimeMillis() + timeInMilliseconds;

                LockGuard guard(mutex);
                while (!(resultReady || exceptionReady) && endTime > util::currentTimeMillis()) {
                    conditionVariable.waitFor(mutex, endTime - util::currentTimeMillis());
                }

                return resultReady || exceptionReady;
            }

            void reset() {
                LockGuard guard(mutex);

                resultReady = false;
                exceptionReady = false;
            }
        private:
            bool resultReady;
            bool exceptionReady;
            ConditionVariable conditionVariable;
            Mutex mutex;
            T sharedObject;
            std::auto_ptr<client::exception::IException> exception;

            Future(const Future& rhs);

            void operator=(const Future& rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Future

