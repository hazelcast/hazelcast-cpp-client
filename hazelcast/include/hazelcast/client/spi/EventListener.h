//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_EVENT_LISTENER
#define HAZELCAST_EVENT_LISTENER

namespace hazelcast {
    namespace client {
        namespace spi {

            class EventListener {
            public:
                virtual ~EventListener() = 0;
            };

            inline EventListener::~EventListener() {

            };
        }

    }
}


#endif //__EventListener_H_
