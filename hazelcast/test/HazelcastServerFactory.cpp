/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/26/13.



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
