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

#include <unordered_map>

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace util {
        class ByteBuffer;
    }
    namespace client {
        namespace protocol {
            template<typename MessageHandler>
            class ClientMessageBuilder {
            public:
                ClientMessageBuilder(MessageHandler &handler)  : message_handler_(handler) {}

                virtual ~ClientMessageBuilder() = default;

                /**
                * @returns true if message is completed, false otherwise
                */
                bool onData(util::ByteBuffer &buffer) {
                    bool isCompleted = false;

                    if (!message_) {
                        message_.reset(new ClientMessage());
                        is_final_frame_ = false;
                        remaining_frame_bytes_ = 0;
                    }

                    if (message_) {
                        message_->fillMessageFrom(buffer, is_final_frame_, remaining_frame_bytes_);
                        isCompleted = is_final_frame_ && remaining_frame_bytes_ == 0;
                        if (isCompleted) {
                            //MESSAGE IS COMPLETE HERE
                            message_->wrap_for_read();
                            isCompleted = true;

                            if (message_->is_flag_set(ClientMessage::UNFRAGMENTED_MESSAGE)) {
                                //MESSAGE IS COMPLETE HERE
                                message_handler_.handleClientMessage(std::move(message_));
                            } else {
                                message_->rd_ptr(ClientMessage::FRAGMENTATION_ID_OFFSET);
                                auto fragmentation_id = message_->get<int64_t>();
                                auto flags = message_->getHeaderFlags();
                                message_->drop_fragmentation_frame();
                                if (ClientMessage::is_flag_set(flags, ClientMessage::BEGIN_FRAGMENT_FLAG)) {
                                    // put the message into the partial messages list
                                    addToPartialMessages(fragmentation_id, message_);
                                } else {
                                    // This is the intermediate frame. Append at the previous message buffer
                                    appendExistingPartialMessage(fragmentation_id, message_,
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
                    partialMessages_[fragmentation_id] = std::move(message);
                }

                /**
                * @returns true if message is completed, false otherwise
                */
                bool appendExistingPartialMessage(int64_t fragmentation_id, std::unique_ptr<ClientMessage> &msg,
                                                  bool is_end_fragment) {
                    bool result = false;

                    auto found = partialMessages_.find(fragmentation_id);
                    if (partialMessages_.end() != found) {
                        found->second->append(std::move(msg));
                        if (is_end_fragment) {
                            // remove from message from map
                            std::shared_ptr<ClientMessage> foundMessage(found->second);

                            partialMessages_.erase(found);

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

                MessageMap partialMessages_;

                std::unique_ptr<ClientMessage> message_;
                MessageHandler &message_handler_;

                bool is_final_frame_;
                size_t remaining_frame_bytes_;
            };
        }
    }
}




