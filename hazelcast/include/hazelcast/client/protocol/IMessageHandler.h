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

#ifndef HAZELCAST_CLIENT_PROTOCOL_IMESSAGEHANDLER_H_
#define HAZELCAST_CLIENT_PROTOCOL_IMESSAGEHANDLER_H_

#include <memory>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                class ClientInvocation;
            }
        }

        namespace connection {
            class Connection;
        }

        namespace protocol {
            class HAZELCAST_API IMessageHandler {
            public:
                virtual ~IMessageHandler() {}

                virtual void handleClientMessage(const std::shared_ptr<spi::impl::ClientInvocation> invocation,
                                                 const std::shared_ptr<ClientMessage> response) = 0;
            };
        }
    }
}




#endif //HAZELCAST_CLIENT_PROTOCOL_IMESSAGEHANDLER_H_
