////
//// Created by sancar koyunlu on 5/9/13.
//// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
//
//#include "SocketPool.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace protocol {
//            SocketPool::SocketPool(Address address, long clientId)
//            :address(address)
//            , clientId(clientId) {
//            }
//
//            SocketPool::~SocketPool() {
//                while (!pool.empty()) {
//                    Socket *socket = pool.front();
//                    pool.pop();
//                    delete socket;
//                }
//
//                map<int, Socket *>::iterator it;
//                for (it = poppedSockets.begin(); it != poppedSockets.end(); it++) {
//                    delete it->second;
//                }
//            }
//
//            Socket *SocketPool::pop() {
//                if (!pool.empty()) {
//                    Socket *socket = pool.front();
//                    poppedSockets[socket->getSocketId()] = socket;
//                    pool.pop();
//                    return socket;
//                } else {
//                    Socket *socket = new Socket(address);
//                    poppedSockets[socket->getSocketId()] = socket;
//                    return socket;
//                }
//            }
//
//            void SocketPool::push(Socket *socket) {
//                pool.push(socket);
//                poppedSockets.erase(socket->getSocketId());
//            }
//
//            SocketPool::SocketPool(SocketPool const & rhs)
//            : address("", "") {
//                //private
//            }
//
//
//        }
//    }
//}
//
//
