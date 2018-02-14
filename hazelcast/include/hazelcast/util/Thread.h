/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 31/03/14.
//
#ifndef HAZELCAST_Thread
#define HAZELCAST_Thread

#include "hazelcast/util/ThreadArgs.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/AtomicBoolean.h"

#include <cstdlib>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/Mutex.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace hazelcast {
    namespace util {

        typedef struct thread_interrupted{

        } thread_interrupted;

        class HAZELCAST_API Thread {
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

            void wakeup();

            void cancel();

            bool join();

        private:
            static DWORD WINAPI controlledThread(LPVOID args);

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3 );

            std::string threadName;
            util::AtomicBoolean isJoined;
			util::AtomicBoolean isInterrupted;
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

            void wakeup();

            void cancel();

            bool join();
        private:
            static void *controlledThread(void *args);

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3 );

            std::string threadName;
            util::AtomicBoolean isJoined;
            pthread_t thread;
            pthread_attr_t attr;
            ConditionVariable wakeupCondition;
            Mutex wakeupMutex;
        };
    }
}

#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Thread


