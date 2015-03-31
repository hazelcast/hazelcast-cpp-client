//
// Created by sancar koyunlu on 18/11/14.
//


#ifndef HAZELCAST_Bits
#define HAZELCAST_Bits

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <memory>

namespace hazelcast {
    namespace util {
        class Bits {
        public:
            /**
            * Short size in bytes
            */
            static const int SHORT_SIZE_IN_BYTES = 2;
            /**
            * Char size in bytes
            */
            static const int CHAR_SIZE_IN_BYTES = 2;
            /**
            * Integer size in bytes
            */
            static const int INT_SIZE_IN_BYTES = 4;
            /**
            * Float size in bytes
            */
            static const int FLOAT_SIZE_IN_BYTES = 4;
            /**
            * Long size in bytes
            */
            static const int LONG_SIZE_IN_BYTES = 8;
            /**
            * Double size in bytes
            */
            static const int DOUBLE_SIZE_IN_BYTES = 8;

            static int readIntB(std::vector<byte> *buffer, unsigned long pos);

        private :
            /**
            * Make the default constructor and the destructor inaccessible from public.
            */
            Bits();

            virtual ~Bits();

            static void swap_4(void *orig, void *target);
        };
    }
}


#endif //HAZELCAST_Bits
