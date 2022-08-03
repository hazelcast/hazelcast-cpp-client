/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/exception/iexception.h"
#include "hazelcast/util/export.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class HAZELCAST_API exception_util {
        public:
            /**
             * Interface used by rethrow/peel to wrap the peeled exception
             */
            class runtime_exception_factory {
            public:
                virtual ~runtime_exception_factory();

                virtual void rethrow(std::exception_ptr throwable, const std::string &message) = 0;
            };

            static void rethrow(std::exception_ptr e);

            static void
            rethrow(std::exception_ptr e, const std::shared_ptr<runtime_exception_factory> &runtime_exception_factory);

        private:
            class hazelcast_exception_factory : public runtime_exception_factory {
                void rethrow(std::exception_ptr throwable, const std::string &message) override;
            };

            static const std::shared_ptr<runtime_exception_factory> &hazelcast_exception_factory();

            static const std::shared_ptr<runtime_exception_factory> hazelcastExceptionFactory;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


