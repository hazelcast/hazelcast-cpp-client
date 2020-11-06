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

#include "hazelcast/util/hazelcast_dll.h"
#include "hazelcast/util/noop.h"
#include "hazelcast/util/type_traits.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class hz_socket;

        namespace connection {
            class ClientConnectionManagerImpl;
        }

        /**
         * An interface that provides the ability to intercept the creation of sockets.
         *
         * \see ClientConfig::setSocketInterceptor
         */
        class HAZELCAST_API socket_interceptor final {
        public:
            /**
             * Set an handler function that will be called with a Socket, 
             * each time the client creates a connection to any Member.
             * \param h a `void` function object that is callable with a single parameter of type `const Socket &`
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            socket_interceptor &on_connect(Handler &&h) & {
                connect_ = std::forward<Handler>(h);
                return *this;
            }

            /**
             * \copydoc SocketInterceptor::on_connect
             */
            template<typename Handler,
                     typename = util::enable_if_rvalue_ref_t<Handler &&>>
            socket_interceptor &&on_connect(Handler &&h) && {
                on_connect(std::forward<Handler>(h));
                return std::move(*this);
            }


        private:
            friend class connection::ClientConnectionManagerImpl;

            using handler_t = std::function<void(const hz_socket &)>;

            handler_t connect_{ util::noop<const hz_socket &> };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


