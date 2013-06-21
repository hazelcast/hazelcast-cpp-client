//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_EVENT_HANDLER
#define HAZELCAST_EVENT_HANDLER


namespace hazelcast {
    namespace client {
        namespace spi {

            class EventHandler {
            public:
                virtual ~EventHandler() = 0;
//                void handle();
            };

            inline EventHandler::~EventHandler() {

            };
        }

    }
}


#endif //__EventHandler_H_
