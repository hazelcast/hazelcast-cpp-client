//
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Batikan Turkmen on 14.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#include "HazelcastInstance.h"
#include "HazelcastInstanceFactory.h"

namespace hazelcast {
    namespace client {
        namespace test {

            HazelcastInstance::HazelcastInstance(HazelcastInstanceFactory& factory)
            :factory(factory)
            , id(factory.getInstanceId())
            , isShutDown(false) {
            };


            void HazelcastInstance::shutdown() {
                if (!isShutDown) {
                    factory.shutdownInstance(id);
                    isShutDown = true;
                }
            };

            HazelcastInstance::~HazelcastInstance() {
                if (!isShutDown) {
                    factory.shutdownInstance(id);
                    isShutDown = true;
                }
            };
        }
    }
}
