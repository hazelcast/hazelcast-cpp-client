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

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/protocol_exceptions.h"

#include <vector>
#include <string>
#include <memory>
#include <stdint.h>
#include <sstream>
#include <boost/endian/conversion.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace util {
class ByteBuffer;
}
namespace client {
namespace serialization {
namespace pimpl {
template<typename Container>
class data_input
{
public:
    explicit data_input(boost::endian::order byte_order, const Container& buf)
      : byte_order_(byte_order)
      , buffer_(buf)
      , pos_(0)
    {}

    data_input(boost::endian::order byte_order,
               const Container& buf,
               int offset)
      : byte_order_(byte_order)
      , buffer_(buf)
      , pos_(offset)
    {}

    inline void read_fully(std::vector<byte>& bytes)
    {
        size_t length = bytes.size();
        check_available(length);
        memcpy(&(bytes[0]), &(buffer_[pos_]), length);
        pos_ += length;
    }

    inline int skip_bytes(int i)
    {
        check_available(i);
        pos_ += i;
        return i;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<byte, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        check_available(1);
        return buffer_[pos_++];
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<char, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        return read<char16_t>();
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<char16_t, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        auto int_value = read<int16_t>();
        return static_cast<char16_t>(int_value);
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<bool, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        return static_cast<bool>(read<byte>());
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<int16_t, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        check_available(util::Bits::SHORT_SIZE_IN_BYTES);
        int16_t result;
        if (byte_order_ == boost::endian::order::big) {
            result = boost::endian::
              endian_load<boost::uint16_t, 2, boost::endian::order::big>(
                &buffer_[pos_]);
        } else {
            result = boost::endian::
              endian_load<boost::int16_t, 2, boost::endian::order::little>(
                &buffer_[pos_]);
        }
        pos_ += util::Bits::SHORT_SIZE_IN_BYTES;
        return result;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<int32_t, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        return read(byte_order_);
    }

    int32_t read(boost::endian::order byte_order)
    {
        check_available(util::Bits::INT_SIZE_IN_BYTES);
        int32_t result;
        if (byte_order == boost::endian::order::big) {
            result = boost::endian::
              endian_load<boost::uint32_t, 4, boost::endian::order::big>(
                &buffer_[pos_]);
        } else {
            result = boost::endian::
              endian_load<boost::int32_t, 4, boost::endian::order::little>(
                &buffer_[pos_]);
        }
        pos_ += util::Bits::INT_SIZE_IN_BYTES;
        return result;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<int64_t, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        check_available(util::Bits::LONG_SIZE_IN_BYTES);
        int64_t result;
        if (byte_order_ == boost::endian::order::big) {
            result = boost::endian::
              endian_load<boost::int64_t, 8, boost::endian::order::big>(
                &buffer_[pos_]);
        } else {
            result = boost::endian::
              endian_load<boost::int64_t, 8, boost::endian::order::little>(
                &buffer_[pos_]);
        }
        pos_ += util::Bits::LONG_SIZE_IN_BYTES;
        return result;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<float, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        union
        {
            int32_t i;
            float f;
        } u;
        u.i = read<int32_t>();
        return u.f;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<double, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        union
        {
            double d;
            int64_t l;
        } u;
        u.l = read<int64_t>();
        return u.d;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<std::string, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        int32_t charCount = read<int32_t>();
        if (util::Bits::NULL_ARRAY == charCount) {
            BOOST_THROW_EXCEPTION(
              exception::io("DataInput::read()", "Null string!!!"));
        }

        return read_string(charCount);
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<hazelcast_json_value,
                   typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        return hazelcast_json_value(read<std::string>());
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<boost::optional<std::string>,
                   typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        int32_t charCount = read<int32_t>();
        if (util::Bits::NULL_ARRAY == charCount) {
            return boost::none;
        }

        return boost::make_optional(read_string(charCount));
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<boost::uuids::uuid, typename std::remove_cv<T>::type>::value,
      T>::type inline read()
    {
        check_available(util::Bits::UUID_SIZE_IN_BYTES);
        boost::uuids::uuid u;
        std::memcpy(&u.data, &buffer_[pos_], util::Bits::UUID_SIZE_IN_BYTES);
        pos_ += util::Bits::UUID_SIZE_IN_BYTES;
        if (byte_order_ == boost::endian::order::little) {
            boost::endian::endian_reverse_inplace<int64_t>(
              *reinterpret_cast<int64_t*>(u.data));
            boost::endian::endian_reverse_inplace<int64_t>(
              *reinterpret_cast<int64_t*>(
                &u.data[util::Bits::LONG_SIZE_IN_BYTES]));
        }

        return u;
    }

    template<typename T>
    typename std::enable_if<
      std::is_same<std::vector<byte>,
                   typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<char>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<bool>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<int16_t>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<int32_t>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<int64_t>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<float>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<double>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<std::string>,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<boost::optional<std::string>>,
                     typename std::remove_cv<T>::type>::value,
      typename boost::optional<T>>::type inline read()
    {
        int32_t len = read<int32_t>();
        if (util::Bits::NULL_ARRAY == len) {
            return boost::none;
        }

        if (len == 0) {
            return boost::none;
        }

        if (len < 0) {
            BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
              "DataInput::readArray",
              (boost::format("Incorrect negative array size found in the byte "
                             "stream. The size is: %1%") %
               len)
                .str()));
        }

        T values;
        values.reserve(len);
        for (int32_t i = 0; i < len; i++) {
            values.push_back(read<typename T::value_type>());
        }
        return boost::make_optional(values);
    }

    int position() { return pos_; }

    void position(int position)
    {
        if (position > pos_) {
            check_available((size_t)(position - pos_));
        }
        pos_ = position;
    }

private:
    boost::endian::order byte_order_;
    const Container& buffer_;
    int pos_;

    void inline check_available(size_t requested_length)
    {
        size_t available = buffer_.size() - pos_;
        if (requested_length > available) {
            BOOST_THROW_EXCEPTION(exception::io(
              "DataInput::checkAvailable",
              (boost::format("Not enough bytes in internal buffer. "
                             "Available:%1% bytes but needed %2% bytes") %
               available % requested_length)
                .str()));
        }
    }

    inline std::string read_string(size_t byte_count)
    {
        check_available(byte_count);
        std::string value(reinterpret_cast<const char*>(&buffer_[pos_]),
                          byte_count);
        pos_ += byte_count;
        return value;
    }
};
} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
