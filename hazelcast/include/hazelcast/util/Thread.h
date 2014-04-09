//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Thread
#define HAZELCAST_Thread

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/ThreadArgs.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/IException.h"
#include <cstdlib>

namespace hazelcast {
    namespace util {

        class Thread {
        public:
            Thread(const std::string &name, void (func)(ThreadArgs &),
                    void *arg0 = NULL,
                    void *arg1 = NULL,
                    void *arg2 = NULL,
                    void *arg3 = NULL)
            : isJoined(false) {
                init(func, arg0, arg1, arg2, arg3, name);
            }

            Thread(void (func)(ThreadArgs &),
                    void *arg0 = NULL,
                    void *arg1 = NULL,
                    void *arg2 = NULL,
                    void *arg3 = NULL)
            : isJoined(false) {
                init(func, arg0, arg1, arg2, arg3, "hz.unnamed");
            }

            static long getThreadID() {
                return long(pthread_self());
            }

            ~Thread() {
                join();
                pthread_attr_destroy(&attr);
            }

            void interrupt() {
                pthread_cancel(thread);
            }

            bool join() {
                if (isJoined) {
                    return true;
                }
                isJoined = true;
                int err = pthread_join(thread, NULL);
                if (EINVAL == err || ESRCH == err || EDEADLK == err) {
                    return false;
                }
                return true;
            }

        private:
            static void *controlledThread(void *args) {
                std::auto_ptr<ThreadArgs> threadArgs((ThreadArgs *) args);
                try {
                    threadArgs->func(*threadArgs);
                } catch(hazelcast::client::exception::IException &){
                } catch(...){
                    ILogger::getLogger().warning(threadArgs->threadName + " is cancelled ");
                    throw;
                }
                return NULL;
            }

            Thread(pthread_t thread):thread(thread) {

            }

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3, const std::string &threadName) {
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
                ThreadArgs *threadArgs = new ThreadArgs;
                threadArgs->arg0 = arg0;
                threadArgs->arg1 = arg1;
                threadArgs->arg2 = arg2;
                threadArgs->arg3 = arg3;
                threadArgs->threadName = threadName;
                threadArgs->func = func;
                pthread_create(&thread, &attr, controlledThread, threadArgs);
            }

            bool isJoined;
            pthread_t thread;
            pthread_attr_t attr;
        };
    }
}

#endif //HAZELCAST_Thread

