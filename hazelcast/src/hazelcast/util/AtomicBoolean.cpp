#include "hazelcast/util/AtomicBoolean.h"

#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        AtomicBoolean::AtomicBoolean():v(true){

        }

        AtomicBoolean::AtomicBoolean(bool v):v(v) {

        }

        bool AtomicBoolean::operator!(){
            LockGuard lockGuard(mutex);
            return !v;
        }

        bool AtomicBoolean::operator ==(bool i){
            LockGuard lockGuard(mutex);
            return v == i;
        }

        bool AtomicBoolean::operator !=(bool i){
            LockGuard lockGuard(mutex);
            return v != i;
        }

        void AtomicBoolean::operator =(bool i){
            LockGuard lockGuard(mutex);
            v = i;
        }

        AtomicBoolean::operator bool(){
            LockGuard lockGuard(mutex);
            return v;
        }

        bool AtomicBoolean::compareAndSet(bool compareValue, bool setValue){
            LockGuard lockGuard(mutex);
            if(compareValue == v){
                v = setValue;
                return true;
            }
            return false;
        }
    }
}
