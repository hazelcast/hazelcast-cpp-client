//
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include <iostream>

namespace hazelcast {
    namespace client {
        namespace test {

            HazelcastServer::HazelcastServer(HazelcastServerFactory& factory)
            :factory(factory)
            , id(factory.getInstanceId())
            , isShutDown(false) {
            }

            bool HazelcastServer::start() {
                bool result = false;

                if (isShutDown) {
                    id = factory.getInstanceId();
                    isShutDown = false;
                    result = true;
                }

                return result;
            }

            bool HazelcastServer::shutdown() {
                bool result = false;
                try{
                    if (!isShutDown) {
                        factory.shutdownInstance(id);
                        isShutDown = true;
                        result = true;
                    }
                }catch(std::exception& e){
                    isShutDown = true;
                    std::cerr << e.what() << std::endl;
                }
                return result;
            }

            HazelcastServer::~HazelcastServer() {
                shutdown();
            }

        }
    }
}

