//
// Created by sancar koyunlu on 8/26/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include <iostream>
#include <string.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace test {

            HazelcastServerFactory::HazelcastServerFactory(const char* hostAddress)
            : address(hostAddress, 6543)
            , socket(address)
            , outputSocketStream(socket)
            , inputSocketStream(socket) {
                if (int error = socket.connect(5000))
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
            }

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

            }

            int HazelcastServerFactory::getInstanceId() {
                outputSocketStream.writeInt(START);
                return inputSocketStream.readInt();
            }


            const std::string& HazelcastServerFactory::getServerAddress() const{
                return address.getHost();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
