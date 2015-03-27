//
// Created by sancar koyunlu on 18/11/14.
//


#ifndef HAZELCAST_Bits
#define HAZELCAST_Bits

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace util {

            /**
            * Sets n-th bit of the byte value
            *
            * @param value byte value
            * @param bit n-th bit
            * @return value
            */
            byte setBit(byte value, int bit);

            /**
            * Clears n-th bit of the byte value
            *
            * @param value byte value
            * @param bit n-th bit
            * @return value
            */
            byte clearBit(byte value, int bit);

            /**
            * Returns true if n-th bit of the value is set, false otherwise
            */
            bool isBitSet(byte value, int bit);

            /**
            *
            * @param buffer to which integer value will be written
            * @pram pos position of the buffer to write integer value
            * @param value integer value that will be written to buffer
            */
            void writeIntToPos(std::vector<byte> & buffer, int pos, int value);

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
}


#endif //HAZELCAST_Bits
