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

/*
 * ClientMessageBuilder.h
 *
 *  Created on: Apr 10, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_MESSAGE_BUILDER
#define HAZELCAST_CLIENT_MESSAGE_BUILDER

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#include <map>
#include <list>
#include <stdint.h>
#include <memory>

namespace hazelcast {
    namespace util {
        class ByteBuffer;
    }
    namespace client {
        namespace connection {
            class Connection;
        }

        namespace protocol {
            class IMessageHandler;

            class ClientMessageBuilder {

            public:
                ClientMessageBuilder(IMessageHandler &service, connection::Connection &connection);

                virtual ~ClientMessageBuilder();

                /**
                * @returns true if message is completed, false otherwise
                */
                bool onData(util::ByteBuffer &buffer);

                /**
                 * Reset the builder so that the message is null pointer
                 */
                void reset();

            private:
                void addToPartialMessages(std::auto_ptr<ClientMessage> message);

                /**
                * @returns true if message is completed, false otherwise
                */
                bool appendExistingPartialMessage(std::auto_ptr<ClientMessage> message);

                typedef std::map<int64_t, ClientMessage * > MessageMap;

                MessageMap partialMessages;

                ClientMessage wrapperMessage;

                std::auto_ptr<ClientMessage> message;

                IMessageHandler &messageHandler;
                connection::Connection &connection;

                int32_t frameLen;
                int32_t offset;
            };
        }
    }
}
#endif //HAZELCAST_CLIENT_MESSAGE_BUILDER

