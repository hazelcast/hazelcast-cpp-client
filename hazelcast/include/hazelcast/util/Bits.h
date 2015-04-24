//
// Created by sancar koyunlu on 18/11/14.
//


#ifndef HAZELCAST_Bits
#define HAZELCAST_Bits

#include "hazelcast/util/HazelcastDll.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace util {


            /**
            * Sets n-th bit of the short value
            *
            * @param value short value
            * @param bit n-th bit
            * @return value
            */
            short setBit(short value, int bit);

            /**
            * Clears n-th bit of the short value
            *
            * @param value short value
            * @param bit n-th bit
            * @return value
            */
            short clearBit(short value, int bit);

            /**
            * Returns true if n-th bit of the value is set, false otherwise
            */
            bool isBitSet(short value, int bit);

            /**
            *
            * @param buffer to which integer value will be written
            * @pram pos position of the buffer to write integer value
            * @param value integer value that will be written to buffer
            */
            void writeIntToPos(std::vector<byte> & buffer, int pos, int value);

        }
    }
}


#endif //HAZELCAST_Bits
