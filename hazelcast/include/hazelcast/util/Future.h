/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/exception/TimeoutException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/client/exception/pimpl/ExceptionHandler.h"
#include "hazelcast/util/ILogger.h"
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

            };

            void set_value(const T& value) {
                LockGuard guard(mutex);
                if (exceptionReady || resultReady) {
                    util::ILogger::getLogger().warning(std::string("Future.set_value should not be called twice"));
                    return;
                }
                sharedObject = value;
                resultReady = true;
                conditionVariable.notify_all();
            };

            void set_exception(const std::string& exceptionName, const std::string& exceptionDetails) {
                LockGuard guard(mutex);
                if (exceptionReady || resultReady) {
                    util::ILogger::getLogger().warning(std::string("Future.set_exception should not be called twice : details ") + exceptionDetails);
                    return;
                }
                this->exceptionName = exceptionName;
                this->exceptionDetails = exceptionDetails;
                exceptionReady = true;
                conditionVariable.notify_all();
            };

            T get() {
                LockGuard guard(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    client::exception::pimpl::ExceptionHandler::rethrow(exceptionName, exceptionDetails);
                }
                conditionVariable.wait(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    client::exception::pimpl::ExceptionHandler::rethrow(exceptionName, exceptionDetails);
                }
                assert(false && "InvalidState");
                return sharedObject;
            };

            T get(time_t timeInSeconds) {
                LockGuard guard(mutex);
                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    client::exception::pimpl::ExceptionHandler::rethrow(exceptionName, exceptionDetails);
                }
                time_t endTime = time(NULL) + timeInSeconds;
                while (!(resultReady || exceptionReady) && endTime > time(NULL)) {
                    conditionVariable.waitFor(mutex, endTime - time(NULL));
                }

                if (resultReady) {
                    return sharedObject;
                }
                if (exceptionReady) {
                    client::exception::pimpl::ExceptionHandler::rethrow(exceptionName, exceptionDetails);
                }
                throw client::exception::TimeoutException("Future::get(timeInSeconds)", "Wait is timed out");
            };

        private:
            bool resultReady;
            bool exceptionReady;
            ConditionVariable conditionVariable;
            Mutex mutex;
            T sharedObject;
            std::string exceptionName;
            std::string exceptionDetails;

            Future(const Future& rhs);

            void operator=(const Future& rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Future

