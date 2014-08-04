//
// Created by sancar koyunlu on 04/08/14.
//


#ifndef HAZELCAST_Closeable
#define HAZELCAST_Closeable


#include "hazelcast/util/HazelcastDll.h"


namespace hazelcast {
    namespace util {

        class HAZELCAST_API Closeable {
        public:
            virtual void close() = 0;

            virtual ~Closeable() {

            };


        };

    }
}
#endif //HAZELCAST_Closeable
