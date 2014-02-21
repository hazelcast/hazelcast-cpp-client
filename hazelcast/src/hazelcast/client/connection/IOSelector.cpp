//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            IOSelector::IOSelector(ConnectionManager &connectionManager)
            :connectionManager(connectionManager) {
                t.tv_sec = 5;
                t.tv_usec = 0;
                isAlive = true;
            };


            IOSelector::~IOSelector() {
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

            void IOSelector::listen() {
                while (isAlive) {
                    try{
                        processListenerQueue();
                        listenInternal();
                    }catch(exception::IException& e){
                        hazelcast::util::ILogger::warning("IOSelector::listen ", e.what());
                    }catch(boost::thread_interrupted&){
                        break;
                    } catch(...){
                        hazelcast::util::ILogger::severe("IOSelector::listen ", "unknown exception");
                    }
                }
            }

            void IOSelector::initListenSocket(util::SocketSet &wakeUpSocketSet) {
                hazelcast::util::ServerSocket serverSocket(0);
				int p = serverSocket.getPort();
				std::string localAddress;
				if(serverSocket.isIpv4())
					localAddress = "127.0.0.1";
				else 
					localAddress = "::1";

				wakeUpSocket.reset(new Socket(Address(localAddress, p)));
                int error = wakeUpSocket->connect();
                if (error == 0) {
					sleepingSocket.reset(serverSocket.accept());
					wakeUpSocketSet.sockets.insert(sleepingSocket.get());
                    wakeUpListenerSocketId = sleepingSocket->getSocketId();
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

            void IOSelector::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    task->run();
                }
            }
        }
    }
}

