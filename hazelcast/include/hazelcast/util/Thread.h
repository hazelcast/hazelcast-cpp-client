//
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Thread
#define HAZELCAST_Thread


#include "hazelcast/util/ThreadArgs.h"
#include <cstdlib>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace hazelcast {
    namespace util {

        typedef struct thread_interrupted{

        } thread_interrupted;

        class Thread {
        public:
            Thread(const std::string &name, void (func)(ThreadArgs &),
                    void *arg0 = NULL,
                    void *arg1 = NULL,
                    void *arg2 = NULL,
                    void *arg3 = NULL);

            Thread(void (func)(ThreadArgs &),
					void *arg0 = NULL,
                    void *arg1 = NULL,
                    void *arg2 = NULL,
                    void *arg3 = NULL);

            static long getThreadID();

            std::string getThreadName() const;

            ~Thread();

			void interruptibleSleep(int seconds);

            void interrupt();

            bool join();

        private:
            static DWORD WINAPI controlledThread(LPVOID args);

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3 );

            std::string threadName;
            bool isJoined;
            HANDLE thread;
			DWORD id;
			ConditionVariable condition;
			Mutex mutex;
        };
    }
}

#else

#include <pthread.h>

namespace hazelcast {
    namespace util {

        class Thread {
        public:
            Thread(const std::string &name, void (func)(ThreadArgs &),
                    void *arg0 = NULL,
                    void *arg1 = NULL,
                    void *arg2 = NULL,
                    void *arg3 = NULL);

            Thread(void (func)(ThreadArgs &),
                    void *arg0 = NULL,
                    void *arg1 = NULL,
                    void *arg2 = NULL,
                    void *arg3 = NULL);

            static long getThreadID();

            std::string getThreadName() const;

            ~Thread();

			void interruptibleSleep(int seconds);

            void interrupt();

            bool join();

        private:
            static void *controlledThread(void *args);

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3 );

            std::string threadName;
            bool isJoined;
            pthread_t thread;
            pthread_attr_t attr;
        };
    }
}

#endif

#endif //HAZELCAST_Thread


