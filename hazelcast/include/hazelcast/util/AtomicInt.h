//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_AtomicInt
#define HAZELCAST_AtomicInt

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        class AtomicInt {
        public:
            AtomicInt():v(0) {

            }

            AtomicInt(int v):v(v) {

            }

            int operator--(int ){
                LockGuard lockGuard(mutex);
                return v--;
            }

            int operator++(int ){
                LockGuard lockGuard(mutex);
                return v++;
            }

            int operator++(){
                LockGuard lockGuard(mutex);
                return ++v;
            }

            void operator =(int i){
                LockGuard lockGuard(mutex);
                v = i;
            }

            operator int(){
                LockGuard lockGuard(mutex);
                return v;
            }

            int operator--(){
                LockGuard lockGuard(mutex);
                return --v;
            }

            bool operator <=(int i){
                LockGuard lockGuard(mutex);
                return v <= i;
            }

            bool operator ==(int i) {
                LockGuard lockGuard(mutex);
                return i == v;
            }

            bool operator !=(int i) {
                LockGuard lockGuard(mutex);
                return i != v;
            }

        private:
            Mutex mutex;
            int v;

            AtomicInt(const AtomicInt &rhs);

            void operator =(const AtomicInt &rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_AtomicInt

