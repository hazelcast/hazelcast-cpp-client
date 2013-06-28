////
//// Created by sancar koyunlu on 5/9/13.
//// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
//#include "Socket.h"
//#include "SerializationService.h"
//
//#ifndef HAZELCAST_CLIENT_SOCKET_POOL
//#define HAZELCAST_CLIENT_SOCKET_POOL
//
//namespace hazelcast {
//    namespace client {
//        namespace protocol {
//            class SocketPool {
//            public:
//                SocketPool(Address address, long clientId);
//
//                ~SocketPool();
//
//            private:
//                Socket *pop();
//
//                void push(Socket *socket);
//
//                SocketPool(const SocketPool&);
//
//                std::queue<Socket *> pool;
//
//                //To avoid danging pointers in case of exception
//                std::map<int/* socketID*/ ,Socket *> poppedSockets;
//                Address address;
//                long clientId;
//            };
//        }
//    }
//}
//
//
//#endif //HAZELCAST_CLIENT_SOCKET_POOL
