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
// Created by sancar koyunlu on 19/08/14.
//

#ifndef HAZELCAST_OwnerConnectionFuture
#define HAZELCAST_OwnerConnectionFuture

#include "hazelcast/util/HazelcastDll.h"
#include <boost/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Address;

        namespace spi {
            class ClientContext;
        }

        namespace connection {
            class Connection;

            class HAZELCAST_API OwnerConnectionFuture {
            public:
                OwnerConnectionFuture(spi::ClientContext&);

                void markAsClosed();

                boost::shared_ptr<Connection> getOrWaitForCreation();

                boost::shared_ptr<Connection> createNew(const Address& address);

                void closeIfAddressMatches(const Address& address);

                void close();
            private:
                boost::shared_ptr<Connection> ownerConnectionPtr;
                spi::ClientContext& clientContext;
            };
        }
    }
}
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_OwnerConnectionFuture
