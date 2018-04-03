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

#ifndef HAZELCAST_CLIENT_SPI_CLIENTEXECUTIONSERVICE_H_
#define HAZELCAST_CLIENT_SPI_CLIENTEXECUTIONSERVICE_H_

#include "hazelcast/client/spi/TaskScheduler.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            /**
             * Executor service for Hazelcast clients.
             * <p>
             * Allows asynchronous execution and scheduling of {@link Runnable} and {@link Callable} commands.
             * <p>
             * Any schedule submit or execute operation runs on internal executors.
             * When user code needs to run getUserExecutor() should be utilized
             */
            class HAZELCAST_API ClientExecutionService : public spi::TaskScheduler {
            public:
                /**
                 * @return executorService that alien (user code) runs on
                 */
                virtual util::ExecutorService &getUserExecutor() = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif

