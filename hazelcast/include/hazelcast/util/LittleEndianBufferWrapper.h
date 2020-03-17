/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by İhsan Demir on 14/05/15.
//

#ifndef HAZELCAST_CLIENT_COMMON_CONTAINERS_LITTLEENDIANBUFFERWRAPPER_H
#define HAZELCAST_CLIENT_COMMON_CONTAINERS_LITTLEENDIANBUFFERWRAPPER_H

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <memory>

#include "hazelcast/util/Bits.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class HAZELCAST_API LittleEndianBufferWrapper {
        public:
            virtual ~LittleEndianBufferWrapper() {
            }

            enum TypeSizes {
                INT8_SIZE = 1,
                UINT8_SIZE = 1,
                INT16_SIZE = 2,
                UINT16_SIZE = 2,
                INT32_SIZE = 4,
                UINT32_SIZE = 4,
                UINT64_SIZE = 8,
                INT64_SIZE = 8
            };

            inline void wrapForWrite(int32_t size, int32_t offset) {
                capacity = size;
                index = offset;
                readOnly = false;
            }

            inline void wrapForRead(int32_t size, int32_t offset) {
                capacity = size;
                index = offset;
                readOnly = true;
            }

            const std::shared_ptr<std::vector<byte> > &getBuffer() const {
                return buffer;
            }

            //---------------------- Getters -------------------------------
            inline std::string getStringUtf8() {
                int32_t len = getInt32();
                assert(checkReadAvailable(len));
                const char *start = (const char *) ix();
                index += len;
                return std::string(start, len);
            }

            inline uint8_t getUint8() {
                assert(checkReadAvailable(UINT8_SIZE));
                return (*buffer)[index++];
            }

            inline int8_t getInt8() {
                assert(checkReadAvailable(INT8_SIZE));
                return (*buffer)[index++];
            }

            inline bool getBoolean() {
                return 0 != getUint8();
            }

            inline uint16_t getUint16() {
                assert(checkReadAvailable(UINT16_SIZE));
                uint16_t result;
                util::Bits::littleEndianToNative2(ix(), &result);
                index += UINT16_SIZE;
                return result;
            }

            inline int16_t getInt16() {
                assert(checkReadAvailable(INT16_SIZE));
                int16_t result;
                util::Bits::littleEndianToNative2(ix(), &result);
                index += INT16_SIZE;
                return result;
            }

            inline uint32_t getUint32() {
                assert(checkReadAvailable(UINT32_SIZE));
                uint32_t result;
                util::Bits::littleEndianToNative4(ix(), &result);
                index += UINT32_SIZE;
                return result;
            }

            inline int32_t getInt32() {
                assert(checkReadAvailable(INT32_SIZE));
                int32_t result;
                util::Bits::littleEndianToNative4(ix(), &result);
                index += INT32_SIZE;
                return result;
            }

            inline uint64_t getUint64() {
                assert(checkReadAvailable(UINT64_SIZE));
                uint64_t result;
                util::Bits::littleEndianToNative8(ix(), &result);
                index += UINT64_SIZE;
                return result;
            }

            inline int64_t getInt64() {
                assert(checkReadAvailable(INT64_SIZE));
                int64_t result;
                util::Bits::littleEndianToNative8(ix(), &result);
                index += INT64_SIZE;
                return result;
            }

            inline const byte *getBytes(int32_t len) {
                assert(checkReadAvailable(len));

                const byte *result = ix();

                index += len;

                return result;
            }

            //---------------------- Setters -------------------------------
            inline void set(uint8_t value) {
                assert(checkWriteAvailable(UINT8_SIZE));
                *(ix()) = value;
                ++index;
            }

            inline void set(int8_t value) {
                assert(checkWriteAvailable(UINT8_SIZE));
                *(ix()) = value;
                ++index;
            }

            inline void set(bool value) {
                assert(checkWriteAvailable(UINT8_SIZE));
                if (value) {
                    set((uint8_t)1);
                } else {
                    set((uint8_t)0);
                }
            }

            inline void set(char value) {
                set((int8_t)value);
            }

            inline void set(uint16_t value) {
                assert(checkWriteAvailable(UINT16_SIZE));
                util::Bits::nativeToLittleEndian2(&value, ix());
                index += UINT16_SIZE;
            }

            inline void set(int16_t value) {
                assert(checkWriteAvailable(INT16_SIZE));
                util::Bits::nativeToLittleEndian2(&value, ix());
                index += INT16_SIZE;
            }

            inline void set(uint32_t value) {
                assert(checkWriteAvailable(UINT32_SIZE));
                util::Bits::nativeToLittleEndian4(&value, ix());
                index += UINT32_SIZE;
            }

            inline void set(int32_t value) {
                assert(checkWriteAvailable(INT32_SIZE));
                util::Bits::nativeToLittleEndian4(&value, ix());
                index += INT32_SIZE;
            }

            inline void set(uint64_t value) {
                assert(checkWriteAvailable(UINT64_SIZE));
                util::Bits::nativeToLittleEndian8(&value, ix());
                index += UINT64_SIZE;
            }

            inline void set(int64_t value) {
                assert(checkWriteAvailable(INT64_SIZE));
                util::Bits::nativeToLittleEndian8(&value, ix());
                index += INT64_SIZE;
            }

            inline void set(const std::string & value) {
                // write len first
                int32_t len = (int32_t)value.length();
                set(len);

                setBytes((byte *)value.c_str(), len);
            }

            inline void setBytes(const byte *value, int32_t len) {
                assert(checkWriteAvailable(len));
                memcpy(ix(), value, (size_t)len);
                index += len;
            }

        protected:
            LittleEndianBufferWrapper(int32_t size) : buffer(new std::vector<byte>(size, 0)), capacity(-1), index(-1),
                                                      readOnly(true) {}

            inline int32_t getIndex() const {
                return index;
            }

            inline int32_t getCapacity() const {
                return capacity;
            }

        protected:
            inline byte *ix() {
                return &(*buffer)[index];
            }

            inline bool checkWriteAvailable(int32_t requestedBytes) const {
                if (readOnly) {
                    return false;
                }

                return checkAvailable(requestedBytes);
            }

            inline bool checkReadAvailable(int32_t requestedBytes) const {
                if (!readOnly) {
                    return false;
                }

                return checkAvailable(requestedBytes);
            }

            inline bool checkAvailable(int32_t requestedBytes) const {
                return index + requestedBytes <= capacity;
            }

            std::shared_ptr<std::vector<byte> > buffer;
            int32_t capacity;
            int32_t index;
            bool readOnly;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_COMMON_CONTAINERS_LITTLEENDIANBUFFERWRAPPER_H
