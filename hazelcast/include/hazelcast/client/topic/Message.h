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

#ifndef HAZELCAST_CLIENT_TOPIC_MESSAGE_H_
#define HAZELCAST_CLIENT_TOPIC_MESSAGE_H_

#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Member;

        namespace topic {
            template <typename E>
            class Message {
            public:
                /**
                 * Returns the published message
                 *
                 * @return the published message object
                 */
                virtual const E *getMessageObject() const = 0;

                /**
                 * Returns the published message with the memory ownership
                 *
                 * @return the published message object with the memory ownership
                 *
                 * Warning: This method can be called only one time, since the message kept internally will no longer
                 * exist after this call, hence the method will only return a null auto_ptr in the second call.
                 */
                virtual std::auto_ptr<E> releaseMessageObject() = 0;

                /**
                 * Return the time when the message is published
                 *
                 * @return the time when the message is published
                 */
                virtual int64_t getPublishTime() const = 0;

                /**
                 * Returns the member that published the message.
                 *
                 * It can be that the member is null if:
                 * <ol>
                 *     <li>the message was send by a client and not a member</li>
                 *     <li>the member that send the message, left the cluster before the message was processed.</li>
                 * </ol>
                 *
                 * @return the member that published the message
                 */
                virtual const Member *getPublishingMember() const = 0;

                /**
                 * @return the name of the topic for which this message is produced
                 */
                virtual const std::string &getSource() const = 0;

                /**
                 * @deprecated Please use getSource instead.
                 *
                 * @return the name of the topic for which this message is produced
                 */
                virtual std::string getName() = 0;

                virtual ~Message() { }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_TOPIC_MESSAGE_H_

