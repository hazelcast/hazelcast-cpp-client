/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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


#pragma once

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <stdint.h>

#if defined(linux) || defined(__linux__) || defined (__GLIBC__) || defined(__GNU__)

	#include <byteswap.h>
	#define bswap16(x) __bswap_16(x)
	#define bswap32(x) __bswap_32(x)
	#define bswap64(x) __bswap_64(x)

	#ifdef __USE_BSD

		#include <endian.h>

		#define letoh16(x) le16toh(x)
		#define letoh32(x) le32toh(x)
		#define letoh64(x) le64toh(x)

		#define betoh16(x) be16toh(x)
		#define betoh32(x) be32toh(x)
		#define betoh64(x) be64toh(x)

		#define BO_ENDIAN_FUNC_DEFINED

	#endif
#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define bswap16(x) OSSwapInt16(x)
#define bswap32(x) OSSwapInt32(x)
#define bswap64(x) OSSwapInt64(x)

#elif defined(_MSC_VER)

	#include <intrin.h>
	#define bswap16(x) _byteswap_ushort(x)
	#define bswap32(x) _byteswap_ulong(x)
	#define bswap64(x) _byteswap_uint64(x)

#endif

#include "hazelcast/util/HazelcastDll.h"
#include <vector>
#include <memory>

namespace hazelcast {
    namespace util {
        class HAZELCAST_API Bits {
        public:
            /**
            * Byte size in bytes
            */
            static constexpr unsigned int BYTE_SIZE_IN_BYTES = 1;
            /**
            * Boolean size in bytes
            */
            static constexpr unsigned int BOOLEAN_SIZE_IN_BYTES = 1;
            /**
            * Char size in bytes
            */
            static constexpr unsigned int CHAR_SIZE_IN_BYTES = 2;
            /**
            * Short size in bytes
            */
            static constexpr unsigned int SHORT_SIZE_IN_BYTES = 2;
            /**
            * Integer size in bytes
            */
            static constexpr unsigned int INT_SIZE_IN_BYTES = 4;
            /**
            * Float size in bytes
            */
            static constexpr unsigned int FLOAT_SIZE_IN_BYTES = 4;
            /**
            * Long size in bytes
            */
            static constexpr unsigned int LONG_SIZE_IN_BYTES = 8;
            /**
            * Double size in bytes
            */
            static constexpr unsigned int DOUBLE_SIZE_IN_BYTES = 8;

            /**
             * for null arrays, this value writen to stream to represent null array size.
             */
            static constexpr int NULL_ARRAY = -1;

            /**
             * Length of the data blocks used by the CPU cache sub-system in bytes.
             */
            static constexpr int CACHE_LINE_LENGTH = 64;

            /**
            * Fills the target with 2 bytes in native bytes order
            * given the source bytes as Little Endian.
            */
            inline static void littleEndianToNative2(const void *source, void *target) {
                #ifdef HZ_BIG_ENDIAN
                    swap_2(source, target);
                #else
                    *(static_cast<uint16_t *>(target)) = *(static_cast<const uint16_t *>(source));
                #endif
            }

            /**
            * Fills the target with 4 bytes in native bytes order
            * given the source bytes as Little Endian.
            */
            inline static void littleEndianToNative4(const void *source, void *target) {
                #ifdef HZ_BIG_ENDIAN
                    swap_4(source, target);
                #else
                    *(static_cast<uint32_t *>(target)) = *(static_cast<const uint32_t *>(source));
                #endif
            }

            /**
            * Fills the target with 8 bytes in native bytes order
            * given the source bytes as Little Endian.
            */
            inline static void littleEndianToNative8(const void *source, void *target) {
                #ifdef HZ_BIG_ENDIAN
                    swap_8(source, target);
                #else
                *(static_cast<uint64_t *>(target)) = *(static_cast<const uint64_t *>(source));
                #endif
            }


            inline static void littleEndianToNativeArray4(int32_t len, int32_t *buffer) {
                #ifdef HZ_BIG_ENDIAN
                for (int i = 0; i < len; ++i) {
                    swapInplace4(buffer);
                    ++buffer;
                }
                #endif
            }

            /**
            * Fills the target with 2 bytes in order Little Endian
            * given the source bytes as native bytes.
            */
            inline static void nativeToLittleEndian2(void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                   swap_2(source, target);
            #else
                *(static_cast<uint16_t *>(target)) = *(static_cast<const uint16_t *>(source));
            #endif
            }

            /**
            * Fills the target with 4 bytes in Little Endian byte order
            * given the source bytes as native byte order.
            */
            inline static void nativeToLittleEndian4(const void *source, void *target) {
                #ifdef HZ_BIG_ENDIAN
                    swap_4(source, target);
                #else
                    *(static_cast<uint32_t *>(target)) = *(static_cast<const uint32_t *>(source));
                #endif
            }

            /**
            * Fills the target with 8 bytes in order Little Endian
            * given the source bytes as native bytes.
            */
            inline static void nativeToLittleEndian8(void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                   swap_8(source, target);
            #else
                *(static_cast<uint64_t *>(target)) = *(static_cast<const uint64_t *>(source));
            #endif
            }

            inline static int32_t readIntB(const std::vector<byte> &buffer, unsigned long pos) {
                #ifdef HZ_BIG_ENDIAN
                    return *((int32_t *) (&buffer[0] + pos));
                #else
                    int32_t result;
                    swap_4(&(buffer[0]) + pos, &result);
                    return result;
                #endif
            }

            // ------------------ BIG ENDIAN Conversions starts ----------------------
            /**
            * Fills the target with 2 bytes in native bytes order
            * given the source bytes as Big Endian.
            */
            inline static void bigEndianToNative2(const void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                   *(static_cast<uint16_t *>(target)) = *(static_cast<const uint16_t *>(source));
            #else
                swap_2(source, target);
            #endif
            }

            /**
            * Fills the target with 4 bytes in native bytes order
            * given the source bytes as Big Endian.
            */
            inline static void bigEndianToNative4(const void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                *(static_cast<uint32_t *>(target)) = *(static_cast<const uint32_t *>(source));
            #else
                swap_4(source, target);
            #endif
            }

            /**
            * Fills the target with 8 bytes in native bytes order
            * given the source bytes as Big Endian.
            */
            inline static void bigEndianToNative8(const void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                *(static_cast<uint64_t *>(target)) = *(static_cast<const uint64_t *>(source));
            #else
                swap_8(source, target);
            #endif
            }

            /**
            * Fills the target with 2 bytes in order Big Endian
            * given the source bytes as native bytes.
            */
            inline static void nativeToBigEndian2(void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                *(static_cast<uint16_t *>(target)) = *(static_cast<const uint16_t *>(source));
            #else
                swap_2(source, target);

            #endif
            }

            /**
            * Fills the target with 4 bytes in Big Endian byte order
            * given the source bytes as native byte order.
            */
            inline static void nativeToBigEndian4(const void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                *(static_cast<uint32_t *>(target)) = *(static_cast<const uint32_t *>(source));
            #else
                swap_4(source, target);
            #endif
            }

            /**
            * Fills the target with 8 bytes in order Big Endian
            * given the source bytes as native bytes.
            */
            inline static void nativeToBigEndian8(void *source, void *target) {
            #ifdef HZ_BIG_ENDIAN
                *(static_cast<uint64_t *>(target)) = *(static_cast<const uint64_t *>(source));
            #else
                swap_8(source, target);
            #endif
            }

            // ------------------ BIG ENDIAN Conversions ends ------------------------

        private :
            inline static void swap_2(const void *orig, void* target) {
                *reinterpret_cast<uint16_t *> (target) =
                        bswap16 (*reinterpret_cast<uint16_t const *> (orig));
            }

            inline static void swapInplace4(void *orig) {
                uint32_t value = * reinterpret_cast<const uint32_t*> (orig);
                swap_4(&value, orig);
            }

            inline static void swap_4 (const void* orig, void* target)
            {
                *reinterpret_cast<uint32_t *> (target) =
                        bswap32 (*reinterpret_cast<uint32_t const *> (orig));
            }

            inline static void swap_8 (const void* orig, void* target)
            {
                *reinterpret_cast<uint64_t *> (target) =
                        bswap64 (*reinterpret_cast<uint64_t const *> (orig));
            }
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


