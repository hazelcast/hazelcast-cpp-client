//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Thread.h"

#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	

namespace hazelcast {
    namespace client {
        namespace connection {

            IOSelector::IOSelector(ConnectionManager &connectionManager)
            :connectionManager(connectionManager) {
                t.tv_sec = 5;
                t.tv_usec = 0;
                isAlive = true;
            }

            void IOSelector::staticListen(util::ThreadArgs &args) {
                IOSelector *inSelector = (IOSelector *) args.arg0;
                inSelector->listen();
            }

            IOSelector::~IOSelector() {
                shutdown();
            }

            void IOSelector::wakeUp() {
                int wakeUpSignal = 9;
                try {
                    wakeUpSocket->send(&wakeUpSignal, sizeof(int));
                } catch(exception::IOException &e) {
                    util::ILogger::getLogger().warning(std::string("Exception at IOSelector::wakeUp ") + e.what());
                    throw e;
                }
            }

            void IOSelector::listen() {
                while (isAlive) {
                    try{
                        processListenerQueue();
                        listenInternal();
                    }catch(exception::IException &e){
                        util::ILogger::getLogger().warning(std::string("Exception at IOSelector::listen() ") + e.what());
                    }
                }
            }

            bool IOSelector::initListenSocket(util::SocketSet &wakeUpSocketSet) {
                hazelcast::util::ServerSocket serverSocket(0);
                int p = serverSocket.getPort();
                std::string localAddress;
                if (serverSocket.isIpv4())
                    localAddress = "127.0.0.1";
                else
                    localAddress = "::1";

                wakeUpSocket.reset(new Socket(Address(localAddress, p)));
                int error = wakeUpSocket->connect(5000);
                if (error == 0) {
                    sleepingSocket.reset(serverSocket.accept());
                    wakeUpSocketSet.insertSocket(sleepingSocket.get());
                    wakeUpListenerSocketId = sleepingSocket->getSocketId();
                    return true;
                } else {
                    util::ILogger::getLogger().severe("IOSelector::initListenSocket " + std::string(strerror(errno)));
                    return false;
                }
            }

            void IOSelector::shutdown() {
                isAlive = false;
            }

            void IOSelector::addTask(ListenerTask *listenerTask) {
                listenerTasks.offer(listenerTask);
            }

            void IOSelector::addSocket(const Socket &socket) {
                socketSet.insertSocket(&socket);
            }

            void IOSelector::removeSocket(const Socket &socket) {
                socketSet.removeSocket(&socket);
            }

            void IOSelector::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    task->run();
                }
            }
        }
    }
}


#pragma warning(pop)



