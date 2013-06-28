//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_THREAD
#define HAZELCAST_THREAD

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>

namespace hazelcast {
    namespace util {
        class Thread {
        public:
            Thread(void *(*runnable)(void *));

            Thread(void *(*runnable)(void *), void *parameters);

            Thread(void *(*runnable)(void *), pthread_attr_t const *, void *parameters);

            void start();

            void join();

            bool join(useconds_t timeout);

//            template<typename V>
//            std::auto_ptr<V> join() {
//                void *result;
//                pthread_join(thread, &result);
//                return *(V *) result;
//            };
//
//            template<typename V>
//            std::auto_ptr<V> join(useconds_t timeout) {
//                useconds_t sleepInterval = 1000 * 100; //100ms
//                while (timeout > 0) {
//                    usleep(sleepInterval);
//                    if (ESRCH == pthread_kill(thread, 0)) {
//                        //Thread not found, means it is finished
//                        void *result;
//                        pthread_join(thread, &result);
//                        return *(V *) result;
//                    }
//                    timeout -= sleepInterval;
//                }
//                pthread_cancel(thread);
//                return std::auto_ptr<V>(NULL);
//            };

            void detach();


        private:
            pthread_t thread;

            void *(*runnable)(void *);

            void *parameters;

            pthread_attr_t const *attributes;

            Thread(const Thread&);

            Thread& operator = (const Thread&);
        };

    }
}


#endif //HAZELCAST_THREAD
