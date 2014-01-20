//
// Created by sancar koyunlu on 8/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "HazelcastInstanceFactory.h"
#include "HazelcastInstance.h"
#include <boost/thread.hpp>

namespace hazelcast {
    namespace client {
        namespace test {

            HazelcastInstanceFactory::HazelcastInstanceFactory()
            : address(HOST, 6543)
            , socket(address)
            , outputSocketStream(socket)
            , inputSocketStream(socket) {
                //system("java -cp ./hazelcast-3.2-SNAPSHOT.jar:.  ClientTCPIPListener & ");
                boost::this_thread::sleep(boost::posix_time::seconds(3));
                if (int error = socket.connect())
                    std::cout << "HazelcastInstanceFactory " << strerror(error) << std::endl;

            }


            HazelcastInstanceFactory::~HazelcastInstanceFactory() {
                outputSocketStream.writeInt(END);
                try {
                    inputSocketStream.readInt();
                    //system("killall -9 java");
                } catch(std::exception &e) {
                    std::cout << e.what() << std::endl;
                    std::cout.flush();
                    //system("killall -9 java");
                }
                //system("killall -9 java");
            }

            void HazelcastInstanceFactory::shutdownInstance(int id) {
                outputSocketStream.writeInt(SHUTDOWN);
                outputSocketStream.writeInt(id);
                int i = inputSocketStream.readInt();
                if (i != OK) {
                    std::cout << "void HazelcastInstanceFactory::shutdownInstance(int id):" << i << std::endl;
                    std::cout.flush();
                    //system("killall -9 java");
                }
            };

            void HazelcastInstanceFactory::shutdownAll() {
                outputSocketStream.writeInt(SHUTDOWN_ALL);
                try {
                    int i = inputSocketStream.readInt();
                    if (i != OK) {
                        std::cout << "void HazelcastInstanceFactory::shutdownAll():" << i << std::endl;
                        std::cout.flush();
                    }
                } catch(std::exception &e) {
                    std::cout << e.what() << std::endl;
                    std::cout.flush();
                    //system("killall -9 java");
                }

            };

            int HazelcastInstanceFactory::getInstanceId() {
                outputSocketStream.writeInt(START);
                return inputSocketStream.readInt();
            }


        }
    }
}
