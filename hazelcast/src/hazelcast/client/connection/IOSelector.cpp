//
// Created by sancar koyunlu on 24/12/13.
//

#include "IOSelector.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"


namespace hazelcast {
    namespace client {
        namespace connection {

            IOSelector::IOSelector() {
                t.tv_sec = 5;
                t.tv_usec = 0;
                isAlive = true;
            };


            IOSelector::~IOSelector() {
                delete wakeUpSocket;
                shutdown();
            }

            void IOSelector::wakeUp() {
                int wakeUpSignal = 9;
                try {
                    wakeUpSocket->send(&wakeUpSignal, sizeof(int));
                } catch(std::exception &e) {
                    std::cerr << e.what() << std::endl;
                    throw e;
                }
            };

            void IOSelector::initListenSocket(util::SocketSet &wakeUpSocketSet) {
                hazelcast::util::ServerSocket serverSocket(0);
                wakeUpSocket = new Socket(Address("127.0.0.1", serverSocket.getPort()));
                int error = wakeUpSocket->connect();
                if (error == 0) {
                    Socket *socket = serverSocket.accept();
                    wakeUpSocketSet.sockets.insert(socket);
                    wakeUpListenerSocketId = socket->getSocketId();
                } else {
                    throw exception::IOException("OListener::init", std::string(strerror(errno)));
                }
            }

            void IOSelector::shutdown() {
                isAlive = false;
            }

            void IOSelector::addTask(ListenerTask *listenerTask) {
                listenerTasks.offer(listenerTask);
            }

            void IOSelector::addSocket(const Socket &socket) {
                socketSet.sockets.insert(&socket);
            }

            void IOSelector::removeSocket(const Socket &socket) {
                socketSet.sockets.erase(&socket);
            }

            void IOSelector::addHandler(int socketId, IOHandler *handler) {
                ioHandlers[socketId] = handler;
            }

            void IOSelector::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    task->run();
                }
            }
        }
    }
}

