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
//
// Created by ihsan demir on 01 June 2016.
//

#ifndef HAZELCAST_CLIENT_IMPL_EXECUTIONCALLBACK_H_
#define HAZELCAST_CLIENT_IMPL_EXECUTIONCALLBACK_H_

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            class IException;
        }
        namespace impl {
            /**
             * ExecutionCallback allows to asynchronously get notified when the execution is completed,
             * either successfully or with error.
             *
             * @param <V> value
             */
            template <typename V>
            class ExecutionCallback {
            public:
                virtual ~ExecutionCallback() { }

                /**
                 * Called when an execution is completed successfully.
                 *
                 * @param response the result of the successful execution
                 */
                virtual void onResponse(V *response) = 0;

                /**
                 * Called when an execution is completed with an error.
                 * @param e the exception that is thrown
                 */
                virtual void onFailure(const exception::IException *e) = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_IMPL_EXECUTIONCALLBACK_H_

