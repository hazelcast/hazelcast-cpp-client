//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Thread
#define HAZELCAST_Thread

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/ThreadArgs.h"
#include <cstdlib>

namespace hazelcast {
    namespace util {

        class Thread {
        public:
            Thread(void (func)(ThreadArgs &), void *arg0 = NULL, void *arg1 = NULL, void *arg2 = NULL, void *arg3 = NULL) {
                ThreadArgs *threadArgs = new ThreadArgs;
                threadArgs->arg0 = arg0;
                threadArgs->arg1 = arg1;
                threadArgs->arg2 = arg2;
                threadArgs->arg3 = arg3;
                threadArgs->func = func;
                pthread_create(&thread, NULL, controlledThread, threadArgs);
            };

            static void *controlledThread(void *args) {
                ThreadArgs *threadArgs = (ThreadArgs *) args;
                threadArgs->func(*threadArgs);
                delete threadArgs;
                return NULL;
            };

            static Thread this_thread() {
                return Thread(pthread_self());
            };

            long getThreadID() {
                return long(pthread_self());
            }


            ~Thread() {
            }

            void interrupt() {
                pthread_cancel(thread);
            };

            bool join() {
                int err = pthread_join(thread, NULL);
                if (EINVAL == err || ESRCH == err || EDEADLK == err) {
                    return false;
                }
                return true;
            };

        private:

            Thread(pthread_t thread):thread(thread) {

            }

            pthread_t thread;
        };
    }
}

#endif //HAZELCAST_Thread
