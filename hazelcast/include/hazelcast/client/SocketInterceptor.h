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

#include <functional>
#include <utility>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/noop.h"
#include "hazelcast/util/type_traits.h"

namespace hazelcast {
    namespace client {
        class Socket;

        namespace connection {
            class ClientConnectionManagerImpl;
        }

        /**
         * An interface that provides the ability to intercept the creation of sockets.
         *
         * \see ClientConfig::setSocketInterceptor
         */
        class HAZELCAST_API SocketInterceptor final {
        public:
            /**
             * Set an handler function that will be called with a Socket, 
             * each time the client creates a connection to any Member.
             * \param h a `void` function object that is callable with a single parameter of type `const Socket &`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            SocketInterceptor &on_connect(Handler &&h) & {
                connect = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc SocketInterceptor::on_connect
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            SocketInterceptor &&on_connect(Handler &&h) && {
                on_connect(std::forward<Handler>(h));
                return std::move(*this);
            }


        private:
            friend class connection::ClientConnectionManagerImpl;

            using handler_t = std::function<void(const Socket &)>;

            handler_t connect{ util::noop<const Socket &> };
        };
    }
}



