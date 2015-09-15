#include "hazelcast/util/AtomicInt.h"

#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        AtomicInt::AtomicInt():v(0) {

        }

        AtomicInt::AtomicInt(int v):v(v) {

        }

        int AtomicInt::operator--(int ){
            LockGuard lockGuard(mutex);
            return v--;
        }

        int AtomicInt::operator++(int ){
            LockGuard lockGuard(mutex);
            return v++;
        }

        int AtomicInt::operator++(){
            LockGuard lockGuard(mutex);
            return ++v;
        }

        void AtomicInt::operator =(int i){
            LockGuard lockGuard(mutex);
            v = i;
        }

        AtomicInt::operator int(){
            LockGuard lockGuard(mutex);
            return v;
        }

        int AtomicInt::operator--(){
            LockGuard lockGuard(mutex);
            return --v;
        }

        bool AtomicInt::operator <=(int i){
            LockGuard lockGuard(mutex);
            return v <= i;
        }

        bool AtomicInt::operator ==(int i) {
            LockGuard lockGuard(mutex);
            return i == v;
        }

        bool AtomicInt::operator !=(int i) {
            LockGuard lockGuard(mutex);
            return i != v;
        }

    }
}
