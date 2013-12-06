//
//  hazelcastInstance.h
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef __hazelcast__hazelcastInstance__
#define __hazelcast__hazelcastInstance__

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace test {

            class HazelcastInstanceFactory;

            class HAZELCAST_API HazelcastInstance {
            public:
                HazelcastInstance(HazelcastInstanceFactory&);

                void shutdown();

                ~HazelcastInstance();

            private:

                HazelcastInstanceFactory& factory;
                int id;
                bool isShutDown;

            };
        }
    }
}

#endif /* defined(__hazelcast__hazelcastInstance__) */
