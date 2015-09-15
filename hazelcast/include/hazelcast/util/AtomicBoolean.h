//
// Created by sancar koyunlu on 01/04/14.
//


#ifndef HAZELCAST_AtomicBoolean
#define HAZELCAST_AtomicBoolean

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Mutex.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API AtomicBoolean {
        public:

            AtomicBoolean();

            AtomicBoolean(bool v);

            bool operator!();

            bool operator ==(bool i);

            bool operator !=(bool i);

            void operator =(bool i);

            operator bool();

            bool compareAndSet(bool compareValue, bool setValue);

        private:
            Mutex mutex;
            bool v;

            AtomicBoolean(const AtomicBoolean &rhs);

            void operator = (const AtomicBoolean &rhs);
        };
    }
}


#endif //HAZELCAST_AtomicBoolean

