//
// Created by sancar koyunlu on 18/11/14.
//

#include "hazelcast/util/Bits.h"

namespace hazelcast {
    namespace util {
        // Bits class implementation
        Bits::Bits() {
        }

        Bits::~Bits() {
        }

        int Bits::readIntB(std::vector<byte> &buffer, unsigned long pos) {
            #ifdef HZ_BIG_ENDIAN
                return *((int *) (&buffer[0] + pos));
            #else
                int result;
                swap_4(&(buffer[0]) + pos, &result);
                return result;
            #endif
        }


        void Bits::swap_4(void *orig, void *target) {
            // TODO: Change with implementation at CDR_BASE.inl from ACE library
            register int x = * reinterpret_cast<const int*> (orig);
            x = (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
            * reinterpret_cast<int *> (target) = x;
        }
    }
}
