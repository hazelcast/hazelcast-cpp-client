//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SocketInterceptor
#define HAZELCAST_SocketInterceptor


#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class Socket;

        /**
         * Base class for socketInterceptor classes to inherit from.
         *
         * @see ClientConfig#setSocketInterceptor(SocketInterceptor *socketInterceptor);
         */
        class SocketInterceptor {
        public:
            /**
             * Will be called with the Socket, each time client creates a connection to any Member.
             */
            virtual void onConnect(const Socket &connectedSocket) = 0;

            virtual ~SocketInterceptor();
        };
    }
}

#endif //HAZELCAST_SocketInterceptor

