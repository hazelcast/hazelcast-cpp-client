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

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API DataOutput {
                public:
                    static constexpr const size_t DEFAULT_SIZE = 4 * 1024;

                    DataOutput(bool dontWrite = false);

                    /**
                     *
                     * @return reference to the internal byte buffer.
                     */
                    inline const std::vector<byte> &toByteArray() const {
                        return outputStream;
                    }

                    /**
                     *
                     * @param bytes The bytes to be appended to the current buffer
                     */
                    inline void appendBytes(const std::vector<byte> &bytes) {
                        outputStream.insert(outputStream.end(), bytes.begin(), bytes.end());
                    }

                    inline void writeZeroBytes(size_t numberOfBytes) {
                        outputStream.insert(outputStream.end(), numberOfBytes, 0);
                    }

                    inline size_t position() {
                        return outputStream.size();
                    }

                    inline void position(size_t newPos) {
                        if (isNoWrite) { return; }
                        if (outputStream.size() < newPos) {
                            outputStream.resize(newPos, 0);
                        }
                    }

                    /**
                    * @param value to be written
                    */
                    template <typename T>
                    typename std::enable_if<std::is_same<byte, typename std::remove_cv<T>::type>::value ||
                            std::is_same<char, typename std::remove_cv<T>::type>::value ||
                            std::is_same<char16_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<bool, typename std::remove_cv<T>::type>::value ||
                            std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<float, typename std::remove_cv<T>::type>::value ||
                            std::is_same<double, typename std::remove_cv<T>::type>::value ||
                            std::is_same<boost::uuids::uuid, typename std::remove_cv<T>::type>::value, void>::type
                    inline write(T) { if (isNoWrite) { return; } }

                    template <typename T>
                    typename std::enable_if<std::is_same<std::string, T>::value, void>::type
                    inline write(const T *value);

                    template <typename T>
                    typename std::enable_if<std::is_same<char, T>::value, void>::type
                    inline write(const T *value);

                    template <typename T>
                    typename std::enable_if<std::is_same<std::string, T>::value ||
                                   std::is_same<HazelcastJsonValue, T>::value>::type
                    inline write(const T &) { if (isNoWrite) { return; }}

                    /**
                    * @param value to vector of values to be written. Only supported built-in values can be written.
                    */
                    template <typename T>
                    typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                                            std::is_same<std::vector<char>, T>::value ||
                                            std::is_same<std::vector<bool>, T>::value ||
                                            std::is_same<std::vector<int16_t>, T>::value ||
                                            std::is_same<std::vector<int32_t>, T>::value ||
                                            std::is_same<std::vector<int64_t>, T>::value ||
                                            std::is_same<std::vector<float>, T>::value ||
                                            std::is_same<std::vector<double>, T>::value ||
                                            std::is_same<std::vector<std::string>, T>::value, void>::type
                    inline write(const T &value);

                    template <typename T>
                    typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                                            std::is_same<std::vector<char>, T>::value ||
                                            std::is_same<std::vector<bool>, T>::value ||
                                            std::is_same<std::vector<int16_t>, T>::value ||
                                            std::is_same<std::vector<int32_t>, T>::value ||
                                            std::is_same<std::vector<int64_t>, T>::value ||
                                            std::is_same<std::vector<float>, T>::value ||
                                            std::is_same<std::vector<double>, T>::value ||
                                            std::is_same<std::vector<std::string>, T>::value, void>::type
                    inline write(const T *value);

                protected:
                    bool isNoWrite;
                    std::vector<byte> outputStream;

                    void inline checkAvailable(int index, int requestedLength) {
                        if (index < 0) {
                            BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("DataOutput::checkAvailable",
                                                                                      (boost::format("Negative pos! -> %1%") % index).str()));
                        }

                        size_t available = outputStream.size() - index;

                        if (requestedLength > (int) available) {
                            BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("DataOutput::checkAvailable",
                                                                                      (boost::format("Cannot write %1% bytes!") % requestedLength).str()));
                        }
                    }

                    /**
                     * Write integer at the provided index. Bounds check is performed.
                     * @param index The index to write the integer
                     * @param value The integer value to be written
                     */
                    inline void writeAt(int index, int32_t value) {
                        if (isNoWrite) { return; }
                        checkAvailable(index, util::Bits::INT_SIZE_IN_BYTES);
                        util::Bits::nativeToBigEndian4(&value, &outputStream[index]);
                    }
                };

                template <>
                HAZELCAST_API void DataOutput::write(byte value);

                template <>
                HAZELCAST_API void DataOutput::write(char value);

                template <>
                HAZELCAST_API void DataOutput::write(char16_t value);

                template <>
                HAZELCAST_API void DataOutput::write(bool value);

                template <>
                HAZELCAST_API void DataOutput::write(int16_t value);

                template <>
                HAZELCAST_API void DataOutput::write(int32_t value);

                template <>
                HAZELCAST_API void DataOutput::write(int64_t value);

                template <>
                HAZELCAST_API void DataOutput::write(float value);

                template <>
                HAZELCAST_API void DataOutput::write(double value);

                template <>
                HAZELCAST_API void DataOutput::write(boost::uuids::uuid value);

                template <>
                HAZELCAST_API void DataOutput::write(const std::string &value);

                template <>
                HAZELCAST_API void DataOutput::write(const HazelcastJsonValue &value);

                template <typename T>
                typename std::enable_if<std::is_same<std::string, T>::value, void>::type
                DataOutput::write(const T *value) {
                    if (isNoWrite) { return; }
                    if (!value) {
                        write<int32_t>(util::Bits::NULL_ARRAY);
                        return;
                    }

                    write(*value);
                }

                template <typename T>
                typename std::enable_if<std::is_same<char, T>::value, void>::type
                DataOutput::write(const T *value) {
                    if (isNoWrite) { return; }
                    if (!value) {
                        write<int32_t>(util::Bits::NULL_ARRAY);
                        return;
                    }
                    write(std::string(value));
                }

                template<typename T>
                typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                                        std::is_same<std::vector<char>, T>::value ||
                                        std::is_same<std::vector<bool>, T>::value ||
                                        std::is_same<std::vector<int16_t>, T>::value ||
                                        std::is_same<std::vector<int32_t>, T>::value ||
                                        std::is_same<std::vector<int64_t>, T>::value ||
                                        std::is_same<std::vector<float>, T>::value ||
                                        std::is_same<std::vector<double>, T>::value ||
                                        std::is_same<std::vector<std::string>, T>::value, void>::type
                DataOutput::write(const T &value) {
                    if (isNoWrite) { return; }
                    int32_t len = (int32_t) value.size();
                    write<int32_t>(len);
                    if (len > 0) {
                        for (auto const &item : value) {
                            write<typename T::value_type>(item);
                        }
                    }
                }

                template<typename T>
                typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                                        std::is_same<std::vector<char>, T>::value ||
                                        std::is_same<std::vector<bool>, T>::value ||
                                        std::is_same<std::vector<int16_t>, T>::value ||
                                        std::is_same<std::vector<int32_t>, T>::value ||
                                        std::is_same<std::vector<int64_t>, T>::value ||
                                        std::is_same<std::vector<float>, T>::value ||
                                        std::is_same<std::vector<double>, T>::value ||
                                        std::is_same<std::vector<std::string>, T>::value, void>::type
                DataOutput::write(const T *value) {
                    if (isNoWrite) { return; }
                    if (!value) {
                        write<int32_t>(util::Bits::NULL_ARRAY);
                        return;
                    }

                    write(*value);
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

