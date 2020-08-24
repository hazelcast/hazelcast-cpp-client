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

#include <memory>
#include <boost/thread/latch.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Sync.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            class IException;
        }
        namespace connection {
            class Connection;

            class HAZELCAST_API ConnectionFuture {
            public:
                ConnectionFuture(Address address, std::shared_ptr<Connection> connectionInProgress,
                                 util::SynchronizedMap<Address, ConnectionFuture> &connectionsInProgress);

                void onSuccess(const std::shared_ptr<Connection> &conn);

                void onFailure(std::exception_ptr t);

                std::shared_ptr<Connection> get();

                const std::shared_ptr<Connection> &getConnectionInProgress() const;

                std::mutex &getLock();

            private:
                std::promise<std::shared_ptr<Connection>> promise_;
                const Address address_;
                std::shared_ptr<Connection> connection_in_progress_;
                util::SynchronizedMap<Address, ConnectionFuture> &connections_in_progress_;
                std::mutex lock_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


