/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <boost/endian/conversion.hpp>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/hazelcast_json_value.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {
class HAZELCAST_API data_output
{
public:
    static constexpr const size_t DEFAULT_SIZE = 4 * 1024;

    data_output(boost::endian::order byte_order, bool dont_write = false);

    /**
     *
     * @return reference to the internal byte buffer.
     */
    const std::vector<byte>& to_byte_array() const;

    /**
     *
     * @param bytes The bytes to be appended to the current buffer
     */
    void append_bytes(const std::vector<byte>& bytes);
    void write_zero_bytes(size_t number_of_bytes);
    size_t position();
    void position(size_t new_pos);

    /**
     * @param value to be written
     */
    template<typename T>
    typename std::enable_if<
      std::is_same<byte, typename std::remove_cv<T>::type>::value ||
        std::is_same<char, typename std::remove_cv<T>::type>::value ||
        std::is_same<char16_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<bool, typename std::remove_cv<T>::type>::value ||
        std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<float, typename std::remove_cv<T>::type>::value ||
        std::is_same<double, typename std::remove_cv<T>::type>::value ||
        std::is_same<boost::uuids::uuid,
                     typename std::remove_cv<T>::type>::value,
      void>::type inline write(T)
    {
        if (is_no_write_) {
            return;
        }
        // TODO sancar looks wierd. Check later.
    }

    template<typename T>
    typename std::enable_if<std::is_same<std::string, T>::value,
                            void>::type inline write(const T* value);

    template<typename T>
    typename std::enable_if<std::is_same<char, T>::value,
                            void>::type inline write(const T* value);

    template<typename T>
    typename std::enable_if<
      std::is_same<std::string, T>::value ||
      std::is_same<hazelcast_json_value, T>::value>::type inline write(const T&)
    {
        if (is_no_write_) {
            return;
        }
        // TODO sancar looks wierd. Check later.
    }

    /**
     * @param value to vector of values to be written. Only supported built-in
     * values can be written.
     */
    template<typename T>
    typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                              std::is_same<std::vector<char>, T>::value ||
                              std::is_same<std::vector<bool>, T>::value ||
                              std::is_same<std::vector<int16_t>, T>::value ||
                              std::is_same<std::vector<int32_t>, T>::value ||
                              std::is_same<std::vector<int64_t>, T>::value ||
                              std::is_same<std::vector<float>, T>::value ||
                              std::is_same<std::vector<double>, T>::value ||
                              std::is_same<std::vector<std::string>, T>::value,
                            void>::type inline write(const T& value);

    template<typename T>
    typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                              std::is_same<std::vector<char>, T>::value ||
                              std::is_same<std::vector<bool>, T>::value ||
                              std::is_same<std::vector<int16_t>, T>::value ||
                              std::is_same<std::vector<int32_t>, T>::value ||
                              std::is_same<std::vector<int64_t>, T>::value ||
                              std::is_same<std::vector<float>, T>::value ||
                              std::is_same<std::vector<double>, T>::value ||
                              std::is_same<std::vector<std::string>, T>::value,
                            void>::type inline write(const T* value);

    //TODO sancar introduced to public before me here https://github.com/hazelcast/hazelcast-cpp-client/commit/e04bb694f02d2d02d5bc598f07d564f8da9de1ac#diff-a88ea399b57da82e30e5a5acd7a2556ea371fba0a9c656bb7e219a179f7f7dfdR132
    void write(int32_t value, boost::endian::order byte_order);

protected:
    boost::endian::order byte_order_;
    bool is_no_write_;
    std::vector<byte> output_stream_;

    void check_available(size_t index, int requested_length);
    void write_boolean_bit_at(size_t index, size_t offset_in_bits, bool value);
    void write_at(size_t index, int8_t value);
    void write_at(size_t index, int16_t value);
    void write_at(size_t index, int32_t value);
    void write_at(size_t index, int64_t value);
    void write_at(size_t index, float value);
    void write_at(size_t index, double value);
    template<typename T>
    void write_int_at(size_t index, T value, int requested_length)
    {
        if (is_no_write_) {
            return;
        }
        check_available(index, requested_length);
        if (byte_order_ == boost::endian::order::big) {
            boost::endian::native_to_big_inplace(value);
        } else {
            boost::endian::native_to_little_inplace(value);
        }
        std::memcpy(&output_stream_[index], &value, requested_length);
    }
};
template<>
HAZELCAST_API void
data_output::write(byte value);

template<>
HAZELCAST_API void
data_output::write(char value);

template<>
HAZELCAST_API void
data_output::write(char16_t value);

template<>
HAZELCAST_API void
data_output::write(bool value);

template<>
HAZELCAST_API void
data_output::write(int16_t value);

template<>
HAZELCAST_API void
data_output::write(int32_t value);

template<>
HAZELCAST_API void
data_output::write(int64_t value);

template<>
HAZELCAST_API void
data_output::write(float value);

template<>
HAZELCAST_API void
data_output::write(double value);

template<>
HAZELCAST_API void
data_output::write(boost::uuids::uuid value);

template<>
HAZELCAST_API void
data_output::write(const std::string& value);

template<>
HAZELCAST_API void
data_output::write(const hazelcast_json_value& value);

template<typename T>
typename std::enable_if<std::is_same<std::string, T>::value, void>::type
data_output::write(const T* value)
{
    if (is_no_write_) {
        return;
    }
    if (!value) {
        write<int32_t>(util::Bits::NULL_ARRAY);
        return;
    }

    write(*value);
}

template<typename T>
typename std::enable_if<std::is_same<char, T>::value, void>::type
data_output::write(const T* value)
{
    if (is_no_write_) {
        return;
    }
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
                          std::is_same<std::vector<std::string>, T>::value,
                        void>::type
data_output::write(const T& value)
{
    if (is_no_write_) {
        return;
    }
    int32_t len = (int32_t)value.size();
    write<int32_t>(len);
    if (len > 0) {
        for (typename T::const_reference item : value) {
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
                          std::is_same<std::vector<std::string>, T>::value,
                        void>::type
data_output::write(const T* value)
{
    if (is_no_write_) {
        return;
    }
    if (!value) {
        write<int32_t>(util::Bits::NULL_ARRAY);
        return;
    }

    write(*value);
}
} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
