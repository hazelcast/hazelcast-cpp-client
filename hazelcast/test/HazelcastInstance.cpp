//
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "HazelcastInstance.h"
#include "HazelcastInstanceFactory.h"
#include <iostream>

namespace hazelcast {
    namespace client {
        namespace test {

            HazelcastInstance::HazelcastInstance(HazelcastInstanceFactory& factory)
            :factory(factory)
            , id(factory.getInstanceId())
            , isShutDown(false) {
            };


            void HazelcastInstance::shutdown() {
				try{
					if (!isShutDown) {
						factory.shutdownInstance(id);
						isShutDown = true;
					}
				}catch(std::exception& e){
					isShutDown = true;
					std::cerr << e.what() << std::endl;
				}
                
            };

            HazelcastInstance::~HazelcastInstance() {
                shutdown();
            };
        }
    }
}
