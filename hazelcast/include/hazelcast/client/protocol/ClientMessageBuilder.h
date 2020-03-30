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
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

            class HAZELCAST_API ClientMessageBuilder {

            public:
                ClientMessageBuilder(connection::Connection &connection);

                virtual ~ClientMessageBuilder();

                /**
                * @returns true if message is completed, false otherwise
                */
                bool onData(util::ByteBuffer &buffer);

            private:
                void addToPartialMessages(std::unique_ptr<ClientMessage> &message);

                /**
                * @returns true if message is completed, false otherwise
                */
                bool appendExistingPartialMessage(std::unique_ptr<ClientMessage> &message);

                typedef std::map<int64_t, std::shared_ptr<ClientMessage> > MessageMap;

                MessageMap partialMessages;

                std::unique_ptr<ClientMessage> message;
                connection::Connection &connection;

                int32_t frameLen;
                int32_t offset;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_MESSAGE_BUILDER

