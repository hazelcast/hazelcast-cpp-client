//
//  hazelcastInstance.h
//  hazelcast
//
//  Created by Batikan Turkmen on 14.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef __hazelcast__hazelcastInstance__
#define __hazelcast__hazelcastInstance__

#include <iostream>

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastInstanceFactory;

            class HazelcastInstance {
            public:
                friend class HazelcastInstanceFactory;

                void shutdown();

                ~HazelcastInstance();

            private:
                HazelcastInstance(HazelcastInstanceFactory&, int);

                HazelcastInstanceFactory& factory;
                int id;
                bool isShutDown;

            };
        }
    }
}

#endif /* defined(__hazelcast__hazelcastInstance__) */
