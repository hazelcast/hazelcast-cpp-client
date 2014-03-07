//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Topic_Message
#define HAZELCAST_Topic_Message

#include "hazelcast/client/Member.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            template <typename E>
            class HAZELCAST_API Message {
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

#endif //HAZELCAST_Message
