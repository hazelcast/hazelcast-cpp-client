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
// Created by sancar koyunlu on 21/08/14.
//
#ifndef HAZELCAST_CallFuture
#define HAZELCAST_CallFuture

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/Address.h"
#include <boost/shared_ptr.hpp>
#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }

        namespace connection {
            class CallPromise;

            class Connection;

            class HAZELCAST_API CallFuture {
            public:
                CallFuture();

                CallFuture(boost::shared_ptr<CallPromise> promise, boost::shared_ptr<Connection> connection, int heartBeatTimeout);

                CallFuture(const CallFuture &rhs);

                CallFuture &operator=(const CallFuture &rhs);

                std::auto_ptr<protocol::ClientMessage> get();

                /**
                 * This method assumes that the caller already has the lock acquired
                 *
                 * @return true if result or exception is ready. false otherwise when timeout expires.
                 *
                 * Does not throw exception
                 */
                bool waitFor(int64_t timeoutInMilliseconds);

                int64_t getCallId() const;

                /**
                 * Call to this method before a successful return from get/waitfor is invalid
                 * @return connection that response came from
                 */
                const Connection &getConnection() const;
            private:
                boost::shared_ptr<CallPromise> promise;
                boost::shared_ptr<Connection> connection;
                int heartBeatTimeout;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CallFuture
