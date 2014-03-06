//
// Created by sancar koyunlu on 8/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include <boost/thread.hpp>

namespace hazelcast {
    namespace client {
        namespace test {

            HazelcastServerFactory::HazelcastServerFactory()
            : address(HOST, 6543)
            , socket(address)
            , outputSocketStream(socket)
            , inputSocketStream(socket) {
                //system("java -cp ./hazelcast-3.2-SNAPSHOT.jar:.  ClientTCPIPListener & ");
                boost::this_thread::sleep(boost::posix_time::seconds(3));
                if (int error = socket.connect())
                    std::cout << "HazelcastServerFactory " << strerror(error) << std::endl;

            }


            HazelcastServerFactory::~HazelcastServerFactory() {
                try {
					outputSocketStream.writeInt(END);
                    inputSocketStream.readInt();
                } catch(std::exception &e) {
                    std::cout << e.what() << std::endl;
                    std::cout.flush();
                }
            }

            void HazelcastServerFactory::shutdownInstance(int id) {
                outputSocketStream.writeInt(SHUTDOWN);
                outputSocketStream.writeInt(id);
                int i = inputSocketStream.readInt();
                if (i != OK) {
                    std::cout << "void HazelcastServerFactory::shutdownInstance(int id):" << i << std::endl;
                }
            };

            void HazelcastServerFactory::shutdownAll() {
                outputSocketStream.writeInt(SHUTDOWN_ALL);
                try {
                    int i = inputSocketStream.readInt();
                    if (i != OK) {
                        std::cout << "void HazelcastServerFactory::shutdownAll():" << i << std::endl;
                        std::cout.flush();
                    }
                } catch(std::exception &e) {
                    std::cout << e.what() << std::endl;
                }

            };

            int HazelcastServerFactory::getInstanceId() {
                outputSocketStream.writeInt(START);
                return inputSocketStream.readInt();
            }


        }
    }
}
