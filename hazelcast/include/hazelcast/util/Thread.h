//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Thread
#define HAZELCAST_Thread

#include "Mutex.h"
#include <cstdlib>

namespace hazelcast {
    namespace util {

        struct ThreadArgs {
            void *clazz;
            void *args;
        };

        class Thread {
        public:
            Thread(void *(func)(void *), void *clazz, void *args) {
                ThreadArgs threadArgs;
                threadArgs.clazz = clazz;
                threadArgs.args = args;
                pthread_create(&thread, NULL, func, args);
            };

            static Thread this_thread(){
                return Thread(pthread_self());
            };

            long getThreadID(){
                return long(pthread_self());
            }

            Thread(void *(func)(void *), void *args) {
                ThreadArgs threadArgs;
                threadArgs.clazz = NULL;
                threadArgs.args = args;
                pthread_create(&thread, NULL, func, args);
            };

            ~Thread(){
            }

            void interrupt(){
                pthread_cancel(thread);
            };

//            void sleep(int seconds){
//                ::sleep(seconds);
//            };

            bool join(){
                int err = pthread_join(thread, NULL);
                if(EINVAL == err || ESRCH == err || EDEADLK == err){
                    return false;
                }
                return true;
            };

        private:

            Thread(pthread_t thread):thread(thread){

            }
//            Mutex sleepMutex;
            pthread_t thread;
        };
    }
}

#endif //HAZELCAST_Thread
