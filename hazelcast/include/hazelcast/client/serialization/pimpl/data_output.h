/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
    inline const std::vector<byte>& to_byte_array() const;

    /**
     *
     * @param bytes The bytes to be appended to the current buffer
     */
    inline void append_bytes(const std::vector<byte>& bytes);

    /**
     *
     * @param number_of_bytes The zeros to be appended to the current buffer
    */
    inline void write_zero_bytes(size_t number_of_bytes);

    /**
     *
     * @return Returns the current position
    */
    inline size_t position() const;

    /**
     *
     * @param new_pos Sets the new position as specified
    */
    inline void position(size_t new_pos);

    /**
     * @param value to be written
     */
    template<typename T>
    typename std::enable_if<
      std::is_same<byte, typename std::remove_cv<T>::type>::value ||
        std::is_same<char, typename std::remove_cv<T>::type>::value ||
        std::is_same<char16_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<bool, typename std::remove_cv<T>::type>::value ||
        std::is_same<int8_t, typename std::remove_cv<T>::type>::value ||
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
    }

    /**
     * @param value to vector of values to be written. Only supported built-in
     * values can be written.
     */
    template<typename T>
    typename std::enable_if<std::is_same<std::vector<byte>, T>::value ||
                              std::is_same<std::vector<char>, T>::value ||
                              std::is_same<std::vector<bool>, T>::value ||
                              std::is_same<std::vector<int8_t>, T>::value ||
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
                              std::is_same<std::vector<int8_t>, T>::value ||
                              std::is_same<std::vector<int16_t>, T>::value ||
                              std::is_same<std::vector<int32_t>, T>::value ||
                              std::is_same<std::vector<int64_t>, T>::value ||
                              std::is_same<std::vector<float>, T>::value ||
                              std::is_same<std::vector<double>, T>::value ||
                              std::is_same<std::vector<std::string>, T>::value,
                            void>::type inline write(const T* value);

    void write(int32_t value, boost::endian::order byte_order);

protected:
    boost::endian::order byte_order_;
    bool is_no_write_;
    std::vector<byte> output_stream_;

    void check_available(size_t index, int requested_length);
    void write_boolean_bit_at(size_t index, size_t offset_in_bits, bool value);

    template<typename T>
    typename std::enable_if<
      std::is_same<float, typename std::remove_cv<T>::type>::value,
      void>::type
    write_at(size_t index, T value)
    {
        union
        {
            float f;
            int32_t i;
        } u;
        u.f = value;
        write_at<int32_t>(index, u.i);
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<double, typename std::remove_cv<T>::type>::value,
      void>::type
    write_at(size_t index, T value)
    {
        union
        {
            double d;
            int64_t l;
        } u;
        u.d = value;
        write_at<int64_t>(index, u.l);
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<int8_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int64_t, typename std::remove_cv<T>::type>::value,
      void>::type
    write_at(size_t index, T value)
    {
        if (is_no_write_) {
            return;
        }
        check_available(index, sizeof(T));
        if (byte_order_ == boost::endian::order::big) {
            boost::endian::native_to_big_inplace(value);
        } else {
            boost::endian::native_to_little_inplace(value);
        }
        std::memcpy(&output_stream_[index], &value, sizeof(T));
    }
};

const std::vector<byte>& data_output::to_byte_array() const
{
    return output_stream_;
}

void data_output::append_bytes(const std::vector<byte>& bytes)
{
    output_stream_.insert(output_stream_.cend(), bytes.cbegin(), bytes.cend());
}

void data_output::write_zero_bytes(size_t number_of_bytes)
{
    output_stream_.insert(output_stream_.cend(), number_of_bytes, 0);
}

size_t data_output::position() const
{
    return output_stream_.size();
}

void data_output::position(size_t new_pos)
{
    if (is_no_write_) {
        return;
    }
    if (output_stream_.size() < new_pos) {
        output_stream_.resize(new_pos, 0);
    }
}

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
data_output::write(int8_t value);

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
                          std::is_same<std::vector<int8_t>, T>::value ||
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
                          std::is_same<std::vector<int8_t>, T>::value ||
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
