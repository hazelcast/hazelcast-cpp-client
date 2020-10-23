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

#pragma once

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#include <unordered_map>
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
        namespace protocol {
            class IMessageHandler;

            template<typename MessageHandler>
            class HAZELCAST_API ClientMessageBuilder {
            public:
                ClientMessageBuilder(MessageHandler &handler)  : message_handler_(handler) {}

                virtual ~ClientMessageBuilder() = default;

                /**
                * @returns true if message is completed, false otherwise
                */
                bool onData(util::ByteBuffer &buffer) {
                    bool isCompleted = false;

                    if (!message) {
                        message.reset(new ClientMessage());
                        is_final_frame = false;
                        remaining_frame_bytes = 0;
                    }

                    if (message) {
                        message->fillMessageFrom(buffer, is_final_frame, remaining_frame_bytes);
                        isCompleted = is_final_frame && remaining_frame_bytes == 0;
                        if (isCompleted) {
                            //MESSAGE IS COMPLETE HERE
                            message->wrap_for_read();
                            isCompleted = true;

                            if (message->is_flag_set(ClientMessage::UNFRAGMENTED_MESSAGE)) {
                                //MESSAGE IS COMPLETE HERE
                                message_handler_.handleClientMessage(std::move(message));
                            } else {
                                message->rd_ptr(ClientMessage::FRAGMENTATION_ID_OFFSET);
                                auto fragmentation_id = message->get<int64_t>();
                                auto flags = message->getHeaderFlags();
                                message->drop_fragmentation_frame();
                                if (ClientMessage::is_flag_set(flags, ClientMessage::BEGIN_FRAGMENT_FLAG)) {
                                    // put the message into the partial messages list
                                    addToPartialMessages(fragmentation_id, message);
                                } else {
                                    // This is the intermediate frame. Append at the previous message buffer
                                    appendExistingPartialMessage(fragmentation_id, message,
                                                                 ClientMessage::is_flag_set(flags,
                                                                                            ClientMessage::END_FRAGMENT_FLAG));
                                }
                            }
                        }
                    }

                    return isCompleted;
                }

            private:
                void addToPartialMessages(int64_t fragmentation_id, std::unique_ptr<ClientMessage> &message) {
                    partialMessages[fragmentation_id] = std::move(message);
                }

                /**
                * @returns true if message is completed, false otherwise
                */
                bool appendExistingPartialMessage(int64_t fragmentation_id, std::unique_ptr<ClientMessage> &msg,
                                                  bool is_end_fragment) {
                    bool result = false;

                    auto found = partialMessages.find(fragmentation_id);
                    if (partialMessages.end() != found) {
                        found->second->append(std::move(msg));
                        if (is_end_fragment) {
                            // remove from message from map
                            std::shared_ptr<ClientMessage> foundMessage(found->second);

                            partialMessages.erase(found);

                            foundMessage->wrap_for_read();
                            message_handler_.handleClientMessage(foundMessage);

                            result = true;
                        }
                    } else {
                        // Should never be here
                        assert(0);
                    }

                    return result;
                }

                typedef std::unordered_map<int64_t, std::shared_ptr<ClientMessage> > MessageMap;

                MessageMap partialMessages;

                std::unique_ptr<ClientMessage> message;
                MessageHandler &message_handler_;

                bool is_final_frame;
                size_t remaining_frame_bytes;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



