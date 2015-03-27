//
// Created by sancar koyunlu on 18/11/14.
//

#include "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace client {
        namespace util {


            byte setBit(byte value, int bit) {
                value |= 1 << bit;
                return value;
            }


            byte clearBit(byte value, int bit) {
                value &= ~(1 << bit);
                return value;
            }


            bool isBitSet(byte value, int bit) {
                return (value & 1 << bit) != 0;
            }


            void writeIntToPos(std::vector<byte>& buffer, int pos, int value) {
                buffer[pos] = (byte)((value >> 24) & 0xFF);
                buffer[pos + 1] = (byte)((value >> 16) & 0xFF);
                buffer[pos + 2] = (byte)((value >> 8) & 0xFF);
                buffer[pos + 3] = (byte)((value) & 0xFF);
            }



            // Bits class implementation
            Bits::Bits() {
            }

            Bits::~Bits() {
            }

            int Bits::readIntB(std::vector<byte> *buffer, unsigned long pos) {
                #ifdef HZ_BIG_ENDIAN
                    return *((int *) (start + pos));
                #else
                    int result;
                    swap_4(&buffer[0] + pos, &result);
                    return result;
                #endif
            }


            void Bits::swap_4(void *orig, void *target) {
                // TODO: Change with implementation at CDR_BASE.inl
                register int x = * reinterpret_cast<const int*> (orig);
                x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
                * reinterpret_cast<int *> (target) = x;
            }
        }
    }
}
