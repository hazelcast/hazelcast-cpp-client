//
// Created by sancar koyunlu on 31/03/14.
//


#ifndef HAZELCAST_AtomicInt
#define HAZELCAST_AtomicInt

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        class HAZELCAST_API AtomicInt {
        public:
            AtomicInt();

            AtomicInt(int v);

            int operator--(int );

            int operator++(int );

            int operator++();

            void operator =(int i);

            operator int();

            int operator--();

            bool operator <=(int i);

            bool operator ==(int i);

            bool operator !=(int i);

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

