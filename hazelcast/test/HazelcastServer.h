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

            class HazelcastServerFactory;

            class HazelcastServer {
            public:
                HazelcastServer(HazelcastServerFactory &);

                /**
                 * @returns true if the server were not started before and it is now started successfully
                 */
                bool start();

                bool shutdown();

                ~HazelcastServer();

            private:

                HazelcastServerFactory & factory;
                int id;
                bool isShutDown;

            };
        }
    }
}

#endif /* defined(__hazelcast__hazelcastInstance__) */

