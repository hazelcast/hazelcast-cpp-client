/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 11/04/14.
//

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/IException.h"
#include <memory>
#include "hazelcast/util/Util.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {

        Thread::Thread(const std::string &name, void (func)(ThreadArgs &),
                void *arg0,
                void *arg1,
                void *arg2,
                void *arg3)
        : threadName(name)
        , isJoined(false)
		, isInterrupted(false){
            init(func, arg0, arg1, arg2, arg3);
        }

        Thread::Thread(void (func)(ThreadArgs &),
                void *arg0,
                void *arg1,
                void *arg2,
                void *arg3)
        : threadName("hz.unnamed")
        , isJoined(false)
		, isInterrupted(false){
            init(func, arg0, arg1, arg2, arg3);

        }

        long Thread::getThreadID() {
            return GetCurrentThreadId();
        }

        Thread::~Thread() {
            if (!isJoined) {
                cancel();
                join();
            }
            CloseHandle(thread);
        }

        void Thread::interruptibleSleep(int seconds){
            LockGuard lock(mutex);
			if(isInterrupted){
				isInterrupted = false;
				throw thread_interrupted();
			}
            bool wokenUpbyInterruption = condition.waitFor(mutex, seconds);
            if(wokenUpbyInterruption && isInterrupted){
				isInterrupted = false;
                throw thread_interrupted();
            }
        }

        void Thread::wakeup() {
			LockGuard lock(mutex);
			condition.notify_all();
        }

        void Thread::cancel() {
			LockGuard lock(mutex);
        	isInterrupted = true;
			condition.notify_all();
        }

        bool Thread::join() {
            if (!isJoined.compareAndSet(false, true)) {
                return true;
            }
            if (id == getThreadID()) {
                // called from inside the thread, deadlock possibility
                return false;
            }

            DWORD err = WaitForSingleObject(thread, INFINITE);
            if (err != WAIT_OBJECT_0) {
                return false;
            }
            isJoined = true;
            return true;
        }

        std::string Thread::getThreadName() const{
            return threadName;
        }

        DWORD WINAPI Thread::controlledThread(LPVOID args) {
            std::auto_ptr<ThreadArgs> threadArgs((ThreadArgs *) args);
            try {
                threadArgs->func(*threadArgs);
            } catch(hazelcast::client::exception::IException & e){
                ILogger::getLogger().warning(threadArgs->currentThread->getThreadName()
                    + " is cancelled with exception " + e.what());
            } catch(thread_interrupted& ){
                ILogger::getLogger().warning(threadArgs->currentThread->getThreadName() + " is cancelled");
            }
            return 1L;
        }

        void Thread::init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3 ) {
            ThreadArgs *threadArgs = new ThreadArgs;
            threadArgs->arg0 = arg0;
            threadArgs->arg1 = arg1;
            threadArgs->arg2 = arg2;
            threadArgs->arg3 = arg3;
            threadArgs->currentThread = this;
            threadArgs->func = func;
            thread = CreateThread(NULL, 0, controlledThread, threadArgs, 0 , &id);
        }
    }
}

#else

#include <sys/errno.h>
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {

        Thread::Thread(const std::string &name, void (func)(ThreadArgs &),
                void *arg0,
                void *arg1,
                void *arg2,
                void *arg3)
        : threadName(name)
        , isJoined(false) {
            init(func, arg0, arg1, arg2, arg3);
        }

        Thread::Thread(void (func)(ThreadArgs &),
                void *arg0,
                void *arg1,
                void *arg2,
                void *arg3)
        : threadName("hz.unnamed")
        , isJoined(false) {
            init(func, arg0, arg1, arg2, arg3);
        }

        long Thread::getThreadID() {
            return (long)pthread_self();
        }

        Thread::~Thread() {
            if (!isJoined) {
                cancel();
                join();
            }
            pthread_attr_destroy(&attr);
        }

        void Thread::interruptibleSleep(int seconds) {
            LockGuard guard(wakeupMutex);
            wakeupCondition.waitFor(wakeupMutex, seconds * 1000);
        }

        std::string Thread::getThreadName() const {
            return threadName;
        }

        void Thread::wakeup() {
            LockGuard guard(wakeupMutex);
            wakeupCondition.notify();
        }

        void Thread::cancel() {
            if (!isJoined) {
                LockGuard guard(wakeupMutex);
                wakeupCondition.notify();

                pthread_cancel(thread);
            }
        }

        bool Thread::join() {
            if (!isJoined.compareAndSet(false, true)) {
                return true;
            }

            if (pthread_equal(thread, pthread_self())) {
                // called from inside the thread, deadlock possibility
                return false;
            }

            int err = pthread_join(thread, NULL);
            if (EINVAL == err || ESRCH == err || EDEADLK == err) {
                isJoined = false;
                return false;
            }
            isJoined = true;
            return true;
        }

        void *Thread::controlledThread(void *args) {
            std::auto_ptr<ThreadArgs> threadArgs((ThreadArgs *) args);
            try {
                threadArgs->func(*threadArgs);
            } catch(hazelcast::client::exception::IException &e){
                ILogger::getLogger().warning(threadArgs->currentThread->getThreadName()
                        + " is cancelled with exception " + e.what());
            } catch(...){
                ILogger::getLogger().warning(threadArgs->currentThread->getThreadName() + " is cancelled ");
                throw;
            }
            return NULL;
        }

        void Thread::init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3) {
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            ThreadArgs *threadArgs = new ThreadArgs;
            threadArgs->arg0 = arg0;
            threadArgs->arg1 = arg1;
            threadArgs->arg2 = arg2;
            threadArgs->arg3 = arg3;
            threadArgs->currentThread = this;
            threadArgs->func = func;
            pthread_create(&thread, &attr, controlledThread, threadArgs);
        }

    }
}

#endif

