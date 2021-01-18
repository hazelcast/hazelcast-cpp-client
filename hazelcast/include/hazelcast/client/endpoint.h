/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <string>
#include <memory>

#include "hazelcast/client/address.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * Endpoint represents a peer in the cluster.
         * It is the client.
         */
        class HAZELCAST_API endpoint {
        public:
            endpoint(boost::uuids::uuid uuid, boost::optional<address> socket_address);

            /**
             * Returns the UUID of this endpoint
             *
             * @return the UUID of this endpoint
             */
            boost::uuids::uuid get_uuid() const;

            /**
             * Returns the socket address for this endpoint.
             *
             * @return the socket address for this endpoint
             */
            const boost::optional<address> &get_socket_address() const;

        private:
            boost::uuids::uuid uuid_;
            boost::optional<address> socket_address_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



