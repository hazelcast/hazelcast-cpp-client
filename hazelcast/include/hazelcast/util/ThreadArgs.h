//
// Created by sancar koyunlu on 02/04/14.
//


#ifndef HAZELCAST_ThreadArgs
#define HAZELCAST_ThreadArgs

#include <string>

namespace hazelcast {
    namespace util {

        class ThreadArgs {
        public:
            void *arg0;
            void *arg1;
            void *arg2;
            void *arg3;
            std::string threadName;
            void (*func)(ThreadArgs &);
        } ;

    }
}


#endif //HAZELCAST_ThreadArgs

