//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/IOListener.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"


namespace hazelcast {
    namespace client {
        namespace connection {

            IOListener::IOListener() {
                t.tv_sec = 5;
                t.tv_usec = 0;
                isAlive = true;
            };


            IOListener::~IOListener() {
                delete wakeUpSocket;
                shutdown();
            }

            void IOListener::wakeUp() {
                int wakeUpSignal = 9;
                try {
                    std::cerr << "send wakeup signal " << std::endl;
                    wakeUpSocket->send(&wakeUpSignal, sizeof(int));
                } catch(std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    throw e;
                }
            };

            void IOListener::initListenSocket(util::SocketSet &wakeUpSocketSet) {
                hazelcast::util::ServerSocket serverSocket(0);
                std::cout << "port " << serverSocket.getPort() << std::endl;
                wakeUpSocket = new Socket(Address("127.0.0.1", serverSocket.getPort()));
                if (wakeUpSocket->connect() == 0) {
                    std::cout << "wakeup socket connection establised" << std::endl;
                    Socket *socket = serverSocket.accept();
                    wakeUpSocketSet.sockets.insert(socket);
                    wakeUpListenerSocketId = socket->getSocketId();
                    std::cout << "wakeup socket insert " << socket->getSocketId() << std::endl;
                } else {
                    throw exception::IOException("OListener::init", "no local socket left");
                }
            }

            void IOListener::shutdown() {
                isAlive = false;
            }

            void IOListener::addTask(ListenerTask *listenerTask) {
                listenerTasks.offer(listenerTask);
            }

            void IOListener::addSocket(const Socket &socket) {
                socketSet.sockets.insert(&socket);
            }

            void IOListener::removeSocket(const Socket &socket) {
                socketSet.sockets.erase(&socket);
            }

            void IOListener::addHandler(int socketId, IOHandler *handler) {
                ioHandlers[socketId] = handler;
            }

            void IOListener::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    std::cout << "pppp " << std::endl;
                    task->run();
                }
            }
        }
    }
}

