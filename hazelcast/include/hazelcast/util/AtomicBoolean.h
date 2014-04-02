//
// Created by sancar koyunlu on 01/04/14.
//


#ifndef HAZELCAST_AtomicBoolean
#define HAZELCAST_AtomicBoolean

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        class AtomicBoolean {
        public:

            AtomicBoolean():v(true){

            }

            AtomicBoolean(bool v):v(v) {

            }

            bool operator!(){
                LockGuard lockGuard(mutex);
                return !v;
            }

            bool operator ==(bool i){
                LockGuard lockGuard(mutex);
                return v == i;
            }

            bool operator !=(bool i){
                LockGuard lockGuard(mutex);
                return v != i;
            }

            void operator =(bool i){
                LockGuard lockGuard(mutex);
                v = i;
            }

            operator bool(){
                LockGuard lockGuard(mutex);
                return v;
            }

            bool compareAndSet(bool compareValue, bool setValue){
                LockGuard lockGuard(mutex);
                if(compareValue == v){
                    v = setValue;
                    return true;
                }
                return false;
            }

        private:
            Mutex mutex;
            bool v;

            AtomicBoolean(const AtomicBoolean &rhs);

            void operator = (const AtomicBoolean &rhs);
        };
    }
}


#endif //HAZELCAST_AtomicBoolean
