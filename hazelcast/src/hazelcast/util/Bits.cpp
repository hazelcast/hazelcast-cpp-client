//
// Created by sancar koyunlu on 18/11/14.
//

#include "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace client {
        namespace util {


            short setBit(short value, int bit) {
                value |= 1 << bit;
                return value;
            }


            short clearBit(short value, int bit) {
                value &= ~(1 << bit);
                return value;
            }


            bool isBitSet(short value, int bit) {
                return (value & 1 << bit) != 0;
            }


            void writeIntToPos(std::vector<byte>& buffer, int pos, int value) {
                buffer[pos] = (byte)((value >> 24) & 0xFF);
                buffer[pos + 1] = (byte)((value >> 16) & 0xFF);
                buffer[pos + 2] = (byte)((value >> 8) & 0xFF);
                buffer[pos + 3] = (byte)((value) & 0xFF);
            }
        }
    }
}
