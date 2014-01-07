//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/OListener.h"
#include "hazelcast/client/connection/OutputHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"
#include <boost/thread/detail/thread.hpp>


namespace hazelcast {
    namespace client {
        namespace connection {

            OListener::OListener() {
                init();
                std::cout << "OListener started " << std::endl;
            }

            OListener::~OListener() {
                std::set<Socket const *>::iterator it = wakeUpSocketSet.sockets.begin();
                delete (*it);
                delete wakeUpSocket;
            };

            void OListener::init() {
                hazelcast::util::ServerSocket serverSocket(0);
                std::cout << "port " << serverSocket.getPort() << std::endl;
                wakeUpSocket = new Socket(Address("127.0.0.1", serverSocket.getPort()));
                if( wakeUpSocket->connect() == 0 ){
                    std::cout << "wakeup socket connection establised" << std::endl;
                    Socket *socket = serverSocket.accept();
                    wakeUpSocketSet.sockets.insert(socket);
                    std::cout << "wakeup socket insert " << socket->getSocketId() << std::endl;
                }else{
                    throw exception::IOException("OListener::init","no local socket left");
                }
            };

            void OListener::wakeUp() {
                int wakeUpSignal = 9;
                try{
                    wakeUpSocket->send(&wakeUpSignal, sizeof(int));
                }catch(std::exception& e){
                    std::cerr << e.what() << std::endl;
                    throw e;
                }
            };

            void OListener::listen() {
                while (isAlive) {
                    processListenerQueue();
                    int n = std::max(wakeUpSocketSet.getHighestSocketId(), socketSet.getHighestSocketId());
                    fd_set write_fds = socketSet.get_fd_set();
                    fd_set wakeUp_fd = wakeUpSocketSet.get_fd_set();
                    int err = select(n, &wakeUp_fd, &write_fds, NULL, &t);
                    if (err == 0) {
                        continue;
                    }
                    if (err == -1) {
                        perror("select");
                        continue;
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = wakeUpSocketSet.sockets.begin(); it != wakeUpSocketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &wakeUp_fd)) {
                            int wakeUpSignal;
                            (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                        }
                    }

                    std::set<Socket const *>::iterator temp;
                    it = socketSet.sockets.begin();
                    std::cout << "---------" << std::endl;
                    while (it != socketSet.sockets.end()) {
                        temp = it++;
                        std::cout << "checking write sockets " << (*temp)->getSocketId() << std::endl;
                        if (FD_ISSET((*temp)->getSocketId(), &write_fds)) {
                            socketSet.sockets.erase(temp);
                            std::cout << "handle write" << std::endl;
                            ioHandlers[(*temp)->getSocketId()]->handle();
                        }
                    }

                }
            }
        }
    }
}