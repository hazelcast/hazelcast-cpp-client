/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <boost/optional.hpp>

#include "hazelcast/util/hazelcast_dll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            class IException;
        }
        /**
         * execution_callback allows to asynchronously get notified when the execution is completed,
         * either successfully or with error.
         *
         * @param <V> value
         */
        template <typename V>
        class execution_callback {
        public:
            virtual ~execution_callback() = default;

            /**
             * Called when an execution is completed successfully.
             *
             * @param response the result of the successful execution
             */
            virtual void on_response(const boost::optional<V> &response) = 0;

            /**
             * Called when an execution is completed with an error.
             * @param e the exception that is thrown
             */
            virtual void on_failure(std::exception_ptr e) = 0;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



