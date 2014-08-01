//
// Created by sancar koyunlu on 11/04/14.
//

#include "hazelcast/util/Thread.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/IException.h"
#include <memory>
#include <hazelcast/util/Util.h>

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
            interrupt();
            join();
            CloseHandle(thread);
        }

        void Thread::interruptibleSleep(int seconds){
            LockGuard lock(mutex);
			if(isInterrupted){
				throw thread_interrupted();
			}
            bool ok = condition.waitFor(mutex, seconds * 1000);
            if(!ok){
                throw thread_interrupted();
            }

        }

        void Thread::interrupt() {
			LockGuard lock(mutex);
			isInterrupted = true;
            condition.notify_all();
        }

        bool Thread::join() {
            if (isJoined) {
                return true;
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
            return long(pthread_self());
        }

        Thread::~Thread() {
            interrupt();
            join();
            pthread_attr_destroy(&attr);
        }

        void Thread::interruptibleSleep(int seconds) {
            util::sleep((unsigned int) seconds);
        }

        std::string Thread::getThreadName() const {
            return threadName;
        }

        void Thread::interrupt() {
            pthread_cancel(thread);
        }

        bool Thread::join() {
            if (isJoined) {
                return true;
            }
            int err = pthread_join(thread, NULL);
            if (EINVAL == err || ESRCH == err || EDEADLK == err) {
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

