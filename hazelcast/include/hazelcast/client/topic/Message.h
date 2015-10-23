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
// Created by sancar koyunlu on 6/27/13.




#ifndef HAZELCAST_Topic_Message
#define HAZELCAST_Topic_Message

#include "hazelcast/client/Member.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            template <typename E>
            class Message {
            public:
                Message(std::string topicName, const E &messageObject, long publishTime, const Member &publishingMember)
                : messageObject(messageObject)
                , publishTime(publishTime)
                , publishingMember(publishingMember)
                , name(topicName) {
                };

                E &getMessageObject() {
                    return messageObject;
                };

                long getPublishTime() {
                    return publishTime;
                };

                Member &getPublishingMember() {
                    return publishingMember;
                };

                std::string getName() {
                    return name;
                }

            private:
                E messageObject;
                long publishTime;
                Member publishingMember;
                std::string name;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Message

