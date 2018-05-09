/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.

#include "hazelcast/util/MurmurHash3.h"

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

#include <stdlib.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

FORCE_INLINE  uint32_t rotl32(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

FORCE_INLINE  uint64_t rotl64(uint64_t x, int8_t r) {
    return (x << r) | (x >> (64 - r));
}

#define    ROTL32(x, y)    rotl32(x,y)
#define ROTL64(x, y)    rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

namespace hazelcast {
    namespace util {
        FORCE_INLINE uint32_t getblock32(const uint32_t *p, int i) {
            return *(p + i);
        }

        FORCE_INLINE uint64_t getblock64(const uint64_t *p, int i) {
            return *(p + i);
        }

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

        FORCE_INLINE uint32_t fmix32(uint32_t h) {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;

            return h;
        }

//----------

        FORCE_INLINE uint64_t fmix64(uint64_t k) {
            k ^= k >> 33;
            k *= BIG_CONSTANT(0xff51afd7ed558ccd);
            k ^= k >> 33;
            k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
            k ^= k >> 33;

            return k;
        }

//-----------------------------------------------------------------------------
        int MurmurHash3_x86_32(const void *key, int len) {
            uint32_t DEFAULT_MURMUR_SEED = 0x01000193;
            uint32_t hash = 0;
            MurmurHash3_x86_32(key , len , DEFAULT_MURMUR_SEED , (void*)&hash);
            return hash;
        }


        void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, void *out) {
            const uint8_t *data = (const uint8_t *)key;
            const int nblocks = len / 4;

            uint32_t h1 = seed;

            const uint32_t c1 = 0xcc9e2d51;
            const uint32_t c2 = 0x1b873593;

            //----------
            // body

            const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);

            for (int i = -nblocks; i; i++) {
                uint32_t k1 = getblock32(blocks, i);

                k1 *= c1;
                k1 = ROTL32(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = ROTL32(h1, 13);
                h1 = h1 * 5 + 0xe6546b64;
            }

            //----------
            // tail

            const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);

            uint32_t k1 = 0;

            switch (len & 3) {
                case 3:
                    k1 ^= tail[2] << 16;
                case 2:
                    k1 ^= tail[1] << 8;
                case 1:
                    k1 ^= tail[0];
                    k1 *= c1;
                    k1 = ROTL32(k1, 15);
                    k1 *= c2;
                    h1 ^= k1;
            };

            //----------
            // finalization

            h1 ^= len;

            h1 = fmix32(h1);

            *(uint32_t *)out = h1;
        }

//-----------------------------------------------------------------------------
    }
}
//-----------------------------------------------------------------------------

