//
// Created by sancar koyunlu on 6/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Topic_Message
#define HAZELCAST_Topic_Message

#include "hazelcast/client/connection/Member.h"
#include "hazelcast/client/impl/EventObject.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            template <typename E>
            class Message : public impl::EventObject {
            public:
                Message(std::string topicName, const E& messageObject, long publishTime, const connection::Member& publishingMember)
                : impl::EventObject(topicName)
                , messageObject(messageObject)
                , publishTime(publishTime)
                , publishingMember(publishingMember) {
                };

                E& getMessageObject() {
                    return messageObject;
                };

                long getPublishTime() {
                    return publishTime;
                };

                connection::Member& getPublishingMember() {
                    return publishingMember;
                };

            private:
                E messageObject;
                long publishTime;
                connection::Member publishingMember;
            };

        }
    }
}

#endif //HAZELCAST_Message
