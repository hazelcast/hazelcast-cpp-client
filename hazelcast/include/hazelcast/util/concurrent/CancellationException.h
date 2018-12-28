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
#ifndef HAZELCAST_UTIL_CONCURRENT_CANCELLATIONEXCEPTION_H_
#define HAZELCAST_UTIL_CONCURRENT_CANCELLATIONEXCEPTION_H_

#include "hazelcast/client/exception/IllegalStateException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        namespace concurrent {
            /**
             * Exception indicating that the result of a value-producing task, cannot be retrieved because the task
             * was cancelled.
             *
             */
            class HAZELCAST_API CancellationException : public client::exception::IllegalStateException {
            public:
                CancellationException(const std::string &source, const std::string &message);

                virtual void raise() const;

                virtual int32_t getErrorCode() const;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_CONCURRENT_CANCELLATIONEXCEPTION_H_
