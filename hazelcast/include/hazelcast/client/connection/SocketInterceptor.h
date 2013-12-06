//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SocketInterceptor
#define HAZELCAST_SocketInterceptor


namespace hazelcast {
    namespace client {
        namespace connection {

            class Socket;

            class HAZELCAST_API SocketInterceptor {
            public:
                virtual void onConnect(const Socket& connectedSocket) = 0;

                virtual ~SocketInterceptor() {
                };
            };
        }
    }
}

#endif //HAZELCAST_SocketInterceptor
