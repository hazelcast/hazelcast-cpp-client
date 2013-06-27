//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ATOMIC_INTEGER
#define HAZELCAST_ATOMIC_INTEGER

namespace hazelcast {
    namespace util {
        class AtomicInteger {
        public:
            AtomicInteger();

            AtomicInteger(int);

            operator int() const;

            AtomicInteger(const AtomicInteger&);

            AtomicInteger& operator = (const AtomicInteger&);

            AtomicInteger& operator = (int);

            AtomicInteger& operator ++();

            AtomicInteger operator ++(int);

            int getAndAdd(int);

            int get() const;

            void set(int);

        private:
            int integer;
        };
    }
}

#endif //HAZELCAST_ATOMIC_INTEGER
