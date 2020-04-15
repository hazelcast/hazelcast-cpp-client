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
#ifndef HAZELCAST_CLIENT_CONNECTION_AUTHENTICATIONFUTURE_H_
#define HAZELCAST_CLIENT_CONNECTION_AUTHENTICATIONFUTURE_H_

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

            class HAZELCAST_API AuthenticationFuture {
            public:
                typedef std::tuple<std::shared_ptr<AuthenticationFuture>, std::shared_ptr<Connection>> FutureTuple;

                AuthenticationFuture(const Address &address,
                                     util::SynchronizedMap<Address, FutureTuple> &connectionsInProgress);

                void onSuccess(const std::shared_ptr<Connection> &connection);

                void onFailure(std::exception_ptr t);

                std::shared_ptr<Connection> get();

            private:
                std::shared_ptr<boost::latch> countDownLatch;
                util::Sync<std::shared_ptr<Connection>> connection;
                util::Sync<std::exception_ptr> throwable;
                const Address address;
                util::SynchronizedMap<Address, FutureTuple> &connectionsInProgress;
                std::atomic_bool isSet;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_CONNECTION_AUTHENTICATIONFUTURE_H_
