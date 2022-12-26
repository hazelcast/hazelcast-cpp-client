/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_SERIALIZATION_PIMPL_COMPACT_H_
#define HAZELCAST_CLIENT_SERIALIZATION_PIMPL_COMPACT_H_
// We are using ifdef guards instead of pragmas because, ide could not recognize
// pragma once for this specific case. compact.h and compact.i.h is included in
// serialization.h not in the beginning but later to avoid cyclic dependency.

#include <boost/thread/future.hpp>
#include <utility>
#include "hazelcast/util/export.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/field_kind.h"
#include "hazelcast/client/serialization/pimpl/compact/default_schema_service.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace spi {
class ClientContext;
}

namespace serialization {
class compact_reader;
class compact_writer;
namespace pimpl {
class schema;
class default_compact_writer;
class schema_writer;
compact_writer HAZELCAST_API
create_compact_writer(pimpl::default_compact_writer* default_compact_writer);
compact_writer HAZELCAST_API
create_compact_writer(pimpl::schema_writer* schema_writer);
compact_reader HAZELCAST_API
create_compact_reader(
  pimpl::compact_stream_serializer& compact_stream_serializer,
  object_data_input& object_data_input,
  const pimpl::schema& schema);
struct field_descriptor;
} // namespace pimpl

/**
 * Classes derived from this class should implement the following static
 * methods:
 *      static std::string type_name() noexpect;
 *      static void write(const T& object, compact_writer &out);
 *      static T read(compact_reader &in);
 *
 * @Beta
 * @since 5.1
 */
struct compact_serializer
{};

/**
 * Provides means of reading compact serialized fields from the binary data.
 * <p>
 * Read operations might throw hazelcast_serialization exception
 * when a field with the given name is not found or there is a type mismatch. On
 * such occasions, one might provide default values to the read methods to
 * return it. Providing default values might be especially useful if the class
 * might evolve in the future, either by adding or removing fields.
 *
 * @Beta
 * @since 5.1
 */
class HAZELCAST_API compact_reader
{
public:
    /**
     * Reads a boolean.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    bool read_boolean(const std::string& field_name);

    /**
     * Reads an 8-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    int8_t read_int8(const std::string& field_name);

    /**
     * Reads a 16-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    int16_t read_int16(const std::string& field_name);

    /**
     * Reads a 32-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    int32_t read_int32(const std::string& field_name);

    /**
     * Reads a 64-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    int64_t read_int64(const std::string& field_name);

    /**
     * Reads a 32-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    float read_float32(const std::string& field_name);

    /**
     * Reads a 64-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    double read_float64(const std::string& field_name);

    /**
     * Reads an UTF-8 encoded string.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     *in  the schema.
     */
    boost::optional<std::string> read_string(const std::string& field_name);

    /**
     * Reads an arbitrary precision and scale floating point number.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<big_decimal> read_decimal(const std::string& field_name);

    /**
     * Reads a time consisting of hour, minute, second, and nano seconds.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<local_time> read_time(const std::string& field_name);

    /**
     * Reads a date consisting of year, month, and day.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<local_date> read_date(const std::string& field_name);

    /**
     * Reads a timestamp consisting of date and time.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<local_date_time> read_timestamp(
      const std::string& field_name);

    /**
     * Reads a timestamp with timezone consisting of date, time and timezone
     * offset.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<offset_date_time> read_timestamp_with_timezone(
      const std::string& field_name);

    /**
     * Reads a compact object
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    template<typename T>
    boost::optional<T> read_compact(const std::string& field_name);

    /**
     * Reads an array of booleans.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<bool>> read_array_of_boolean(
      const std::string& field_name);

    /**
     * Reads an array of 8-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<int8_t>> read_array_of_int8(
      const std::string& field_name);

    /**
     * Reads an array of 16-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<int16_t>> read_array_of_int16(
      const std::string& field_name);

    /**
     * Reads an array of 32-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<int32_t>> read_array_of_int32(
      const std::string& field_name);

    /**
     * Reads an array of 64-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<int64_t>> read_array_of_int64(
      const std::string& field_name);

    /**
     * Reads an array of 32-bit IEEE 754 floating point numbers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<float>> read_array_of_float32(
      const std::string& field_name);

    /**
     * Reads an array of 64-bit IEEE 754 floating point numbers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<double>> read_array_of_float64(
      const std::string& field_name);

    /**
     * Reads an array of UTF-8 encoded strings.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<std::string>>>
    read_array_of_string(const std::string& field_name);

    /**
     * Reads an array of arbitrary precision and scale floating point numbers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<big_decimal>>>
    read_array_of_decimal(const std::string& field_name);

    /**
     * Reads an array of times consisting of hour, minute, second, and
     * nanoseconds.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<local_time>>>
    read_array_of_time(const std::string& field_name);

    /**
     * Reads an array of dates consisting of year, month, and day.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<local_date>>>
    read_array_of_date(const std::string& field_name);

    /**
     * Reads an array of timestamps consisting of date and time.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<local_date_time>>>
    read_array_of_timestamp(const std::string& field_name);

    /**
     * Reads an array of timestamps with timezone consisting of date, time and
     * timezone offset.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<offset_date_time>>>
    read_array_of_timestamp_with_timezone(const std::string& field_name);

    /**
     * Reads an array of compact objects.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    template<typename T>
    boost::optional<std::vector<boost::optional<T>>> read_array_of_compact(
      const std::string& field_name);

    /**
     * Reads a nullable boolean.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<bool> read_nullable_boolean(const std::string& field_name);

    /**
     * Reads a nullable 8-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<int8_t> read_nullable_int8(const std::string& field_name);

    /**
     * Reads a nullable 16-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<int16_t> read_nullable_int16(const std::string& field_name);

    /**
     * Reads a nullable 32-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<int32_t> read_nullable_int32(const std::string& field_name);

    /**
     * Reads a nullable 64-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<int64_t> read_nullable_int64(const std::string& field_name);

    /**
     * Reads a nullable 32-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<float> read_nullable_float32(const std::string& field_name);

    /**
     * Reads a nullable 64-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<double> read_nullable_float64(
      const std::string& field_name);

    /**
     * Reads a nullable array of nullable booleans.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<bool>>>
    read_array_of_nullable_boolean(const std::string& field_name);

    /**
     * Reads a nullable array of nullable 8-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<int8_t>>>
    read_array_of_nullable_int8(const std::string& field_name);

    /**
     * Reads a nullable array of nullable 16-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<int16_t>>>
    read_array_of_nullable_int16(const std::string& field_name);

    /**
     * Reads a nullable array of nullable 32-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<int32_t>>>
    read_array_of_nullable_int32(const std::string& field_name);

    /**
     * Reads a nullable array of nullable 64-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<int64_t>>>
    read_array_of_nullable_int64(const std::string& field_name);

    /**
     * Reads a nullable array of nullable 32-bit IEEE 754 floating point
     * numbers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<float>>>
    read_array_of_nullable_float32(const std::string& field_name);

    /**
     * Reads a nullable array of nullable 64-bit IEEE 754 floating point
     * numbers.
     *
     * @param field_name name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in the
     * schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::vector<boost::optional<double>>>
    read_array_of_nullable_float64(const std::string& field_name);

private:
    compact_reader(pimpl::compact_stream_serializer& compact_stream_serializer,
                   object_data_input& object_data_input,
                   const pimpl::schema& schema);
    friend compact_reader pimpl::create_compact_reader(
      pimpl::compact_stream_serializer& compact_stream_serializer,
      object_data_input& object_data_input,
      const pimpl::schema& schema);
    template<typename T>
    T read_primitive(const std::string& field_name,
                     field_kind kind,
                     field_kind nullable_kind,
                     const std::string& method_suffix);
    template<typename T>
    T read_primitive(const pimpl::field_descriptor& field_descriptor);
    bool is_field_exists(const std::string& field_name, field_kind kind) const;
    const pimpl::field_descriptor& get_field_descriptor(
      const std::string& field_name) const;
    const pimpl::field_descriptor& get_field_descriptor(
      const std::string& field_name,
      field_kind field_kind) const;
    template<typename T>
    boost::optional<T> read_variable_size(
      const pimpl::field_descriptor& field_descriptor);
    template<typename T>
    boost::optional<T> read_variable_size(const std::string& field_name,
                                          field_kind field_kind);
    template<typename T>
    T read_variable_size_as_non_null(
      const pimpl::field_descriptor& field_descriptor,
      const std::string& field_name,
      const std::string& method_suffix);
    template<typename T>
    typename std::enable_if<
      std::is_same<bool, typename std::remove_cv<T>::type>::value ||
        std::is_same<int8_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<float, typename std::remove_cv<T>::type>::value ||
        std::is_same<double, typename std::remove_cv<T>::type>::value ||
        std::is_same<std::string, typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<int8_t>,
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
        std::is_same<std::vector<boost::optional<std::string>>,
                     typename std::remove_cv<T>::type>::value,
      typename boost::optional<T>>::type
    read();
    template<typename T>
    typename std::enable_if<
      std::is_base_of<compact_serializer, hz_serializer<T>>::value,
      typename boost::optional<T>>::type
    read();
    template<typename T>
    typename std::enable_if<
      std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value,
      typename boost::optional<T>>::type
    read();
    template<typename T>
    typename std::enable_if<
      std::is_same<std::vector<boost::optional<bool>>,
                   typename std::remove_cv<T>::type>::value,
      typename boost::optional<T>>::type
    read();
    template<typename T>
    typename std::enable_if<
      std::is_same<big_decimal, typename std::remove_cv<T>::type>::value ||
        std::is_same<local_time, typename std::remove_cv<T>::type>::value ||
        std::is_same<local_date, typename std::remove_cv<T>::type>::value ||
        std::is_same<local_date_time,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<offset_date_time, typename std::remove_cv<T>::type>::value,
      typename boost::optional<T>>::type
    read();
    template<typename T>
    boost::optional<T> read_array_of_primitive(
      const std::string& field_name,
      field_kind kind,
      field_kind nullable_kind,
      const std::string& method_suffix);
    template<typename T>
    boost::optional<std::vector<boost::optional<T>>>
    read_array_of_variable_size(
      const pimpl::field_descriptor& field_descriptor);
    template<typename T>
    boost::optional<T> read_nullable_array_as_primitive_array(
      const pimpl::field_descriptor& field_descriptor,
      const std::string& field_name,
      const std::string& method_suffix);
    using offset_func = std::function<
      int32_t(serialization::object_data_input&, uint32_t, uint32_t)>;
    static const offset_func BYTE_OFFSET_READER;
    static const offset_func SHORT_OFFSET_READER;
    static const offset_func INT_OFFSET_READER;
    template<typename T>
    boost::optional<T> read_nullable_primitive(const std::string& field_name,
                                               field_kind kind,
                                               field_kind nullable_kind);
    template<typename T>
    boost::optional<std::vector<boost::optional<T>>> read_array_of_nullable(
      const std::string& field_name,
      field_kind kind,
      field_kind nullable_kind);
    template<typename T>
    boost::optional<std::vector<boost::optional<T>>>
    read_primitive_array_as_nullable_array(
      const pimpl::field_descriptor& field_descriptor);
    static std::function<
      int32_t(serialization::object_data_input&, uint32_t, uint32_t)>
    get_offset_reader(int32_t data_length);
    static exception::hazelcast_serialization unexpected_null_value_in_array(
      const std::string& field_name,
      const std::string& method_suffix);
    exception::hazelcast_serialization unknown_field(
      const std::string& field_name) const;
    exception::hazelcast_serialization unexpected_field_kind(
      field_kind kind,
      const std::string& field_name) const;
    static exception::hazelcast_serialization unexpected_null_value(
      const std::string& field_name,
      const std::string& method_suffix);
    size_t read_fixed_size_position(
      const pimpl::field_descriptor& field_descriptor) const;
    int32_t read_var_size_position(
      const pimpl::field_descriptor& field_descriptor) const;
    pimpl::compact_stream_serializer& compact_stream_serializer;
    serialization::object_data_input& object_data_input;
    const pimpl::schema& schema;
    int32_t data_start_position;
    size_t variable_offsets_position;
    /**
     * Returns the offset of the variable-size field at the given index.
     * @param serialization::object_data_input& Input to read the offset from.
     * @param uint32_t start of the variable-size field offsets
     *                           section of the input.
     * @param uint32_t index of the field.
     * @return The offset.
     */
    std::function<
      int32_t(serialization::object_data_input&, uint32_t, uint32_t)>
      get_offset;
};

/**
 *  Provides means of writing compact serialized fields to the binary data.
 *
 * @Beta
 * @since 5.1
 */
class HAZELCAST_API compact_writer
{
public:
    /**
     * Writes a boolean value.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_boolean(const std::string& field_name, bool value);

    /**
     * Writes an 8-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_int8(const std::string& field_name, int8_t value);

    /**
     *  Writes a 16-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_int16(const std::string& field_name, int16_t value);

    /**
     * Writes a 32-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_int32(const std::string& field_name, int32_t value);

    /**
     * Writes a 64-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_int64(const std::string& field_name, int64_t value);

    /**
     * Writes a 32-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_float32(const std::string& field_name, float value);

    /**
     * Writes a 64-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_float64(const std::string& field_name, double value);

    /**
     * Writes an UTF-8 encoded string.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_string(const std::string& field_name,
                      const boost::optional<std::string>& value);

    /**
     * Writes an arbitrary precision and scale floating point number.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_decimal(const std::string& field_name,
                       const boost::optional<big_decimal>& value);

    /**
     *  Writes a time consisting of hour, minute, second, and nanoseconds.
     *
     *  @param field_name name of the field.
     *  @param value to be written.
     */
    void write_time(const std::string& field_name,
                    const boost::optional<local_time>& value);

    /**
     * Writes a date consisting of year, month, and day.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_date(const std::string& field_name,
                    const boost::optional<local_date>& value);

    /**
     * Writes a timestamp consisting of date and time.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_timestamp(const std::string& field_name,
                         const boost::optional<local_date_time>& value);

    /**
     * Writes a timestamp with timezone consisting of date, time and timezone
     * offset.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_timestamp_with_timezone(
      const std::string& field_name,
      const boost::optional<offset_date_time>& value);

    /**
     * Writes a nested compact object.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    template<typename T>
    void write_compact(const std::string& field_name,
                       const boost::optional<T>& value);

    /**
     * Writes an array of booleans.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_boolean(
      const std::string& field_name,
      const boost::optional<std::vector<bool>>& value);

    /**
     * Writes an array of 8-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_int8(const std::string& field_name,
                             const boost::optional<std::vector<int8_t>>& value);

    /**
     * Writes an array of 16-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_int16(
      const std::string& field_name,
      const boost::optional<std::vector<int16_t>>& value);

    /**
     * Writes an array of 32-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_int32(
      const std::string& field_name,
      const boost::optional<std::vector<int32_t>>& value);

    /**
     * Writes an array of 64-bit two's complement signed integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_int64(
      const std::string& field_name,
      const boost::optional<std::vector<int64_t>>& value);

    /**
     * Writes an array of 32-bit IEEE 754 floating point numbers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_float32(
      const std::string& field_name,
      const boost::optional<std::vector<float>>& value);

    /**
     * Writes an array of 64-bit IEEE 754 floating point numbers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_float64(
      const std::string& field_name,
      const boost::optional<std::vector<double>>& value);

    /**
     * Writes an array of UTF-8 encoded strings.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_string(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<std::string>>>& value);

    /**
     * Writes an array of arbitrary precision and scale floating point numbers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_decimal(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<big_decimal>>>& value);

    /**
     * Writes an array of times consisting of hour, minute, second, and nano
     * seconds.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_time(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<local_time>>>& value);

    /**
     * Writes an array of dates consisting of year, month, and day.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_date(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<local_date>>>& value);

    /**
     * Writes an array of timestamps consisting of date and time.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_timestamp(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<local_date_time>>>&
        value);

    /**
     * Writes an array of timestamps with timezone consisting of date, time and
     * timezone offset.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_timestamp_with_timezone(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<offset_date_time>>>&
        value);

    /**
     * Writes an array of nested compact objects.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    template<typename T>
    void write_array_of_compact(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<T>>>& value);

    /**
     * Writes a nullable boolean.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_boolean(const std::string& field_name,
                                const boost::optional<bool>& value);

    /**
     * Writes a nullable 8-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_int8(const std::string& field_name,
                             const boost::optional<int8_t>& value);

    /**
     * Writes a nullable 16-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_int16(const std::string& field_name,
                              const boost::optional<int16_t>& value);

    /**
     * Writes a nullable 32-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_int32(const std::string& field_name,
                              const boost::optional<int32_t>& value);

    /**
     * Writes a nullable 64-bit two's complement signed integer.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_int64(const std::string& field_name,
                              const boost::optional<int64_t>& value);

    /**
     * Writes a nullable 32-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_float32(const std::string& field_name,
                                const boost::optional<float>& value);

    /**
     * Writes a nullable 64-bit IEEE 754 floating point number.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_nullable_float64(const std::string& field_name,
                                const boost::optional<double>& value);

    /**
     * Writes a nullable array of nullable booleans.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_boolean(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<bool>>>& value);

    /**
     * Writes a nullable array of nullable 8-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_int8(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int8_t>>>& value);

    /**
     * Writes a nullable array of nullable 16-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_int16(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int16_t>>>& value);

    /**
     * Writes a nullable array of nullable 32-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_int32(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int32_t>>>& value);

    /**
     * Writes a nullable array of nullable 64-bit two's complement signed
     * integers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_int64(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int64_t>>>& value);

    /**
     * Writes a nullable array of nullable 32-bit IEEE 754 floating point
     * numbers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_float32(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<float>>>& value);

    /**
     * Writes a nullable array of nullable 64-bit IEEE 754 floating point
     * numbers.
     *
     * @param field_name name of the field.
     * @param value to be written.
     */
    void write_array_of_nullable_float64(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<double>>>& value);

private:
    friend compact_writer pimpl::create_compact_writer(
      pimpl::default_compact_writer* default_compact_writer);
    friend compact_writer pimpl::create_compact_writer(
      pimpl::schema_writer* schema_writer);
    explicit compact_writer(
      pimpl::default_compact_writer* default_compact_writer);
    explicit compact_writer(pimpl::schema_writer* schema_writer);
    pimpl::default_compact_writer* default_compact_writer;
    pimpl::schema_writer* schema_writer;
};

namespace pimpl {

namespace offset_reader {
/**
 * Offset of the null fields.
 */
constexpr int32_t NULL_OFFSET = -1;
/**
 * Range of the offsets that can be represented by a single byte
 * and can be read with offset_reader::BYTE_OFFSET_READER.
 */
constexpr int32_t BYTE_OFFSET_READER_RANGE = INT8_MAX - INT8_MIN;

/**
 * Range of the offsets that can be represented by two bytes
 * and can be read with offset_reader::SHORT_OFFSET_READER.
 */
constexpr int32_t SHORT_OFFSET_READER_RANGE = INT16_MAX - INT16_MIN;

} // namespace offset_reader

class HAZELCAST_API default_compact_writer
{
public:
    default_compact_writer(compact_stream_serializer& compact_stream_serializer,
                           object_data_output& object_data_output,
                           const schema& schema);
    void write_boolean(const std::string& field_name, bool value);
    void write_int8(const std::string& field_name, int8_t value);
    void write_int16(const std::string& field_name, int16_t value);
    void write_int32(const std::string& field_name, int32_t value);
    void write_int64(const std::string& field_name, int64_t value);
    void write_float32(const std::string& field_name, float value);
    void write_float64(const std::string& field_name, double value);
    void write_string(const std::string& field_name,
                      const boost::optional<std::string>& value);
    void write_decimal(const std::string& field_name,
                       const boost::optional<big_decimal>& value);
    void write_time(const std::string& field_name,
                    const boost::optional<local_time>& value);
    void write_date(const std::string& field_name,
                    const boost::optional<local_date>& value);
    void write_timestamp(const std::string& field_name,
                         const boost::optional<local_date_time>& value);
    void write_timestamp_with_timezone(
      const std::string& field_name,
      const boost::optional<offset_date_time>& value);
    template<typename T>
    void write_compact(const std::string& field_name,
                       const boost::optional<T>& value);
    void write_array_of_boolean(
      const std::string& field_name,
      const boost::optional<std::vector<bool>>& value);
    void write_array_of_int8(const std::string& field_name,
                             const boost::optional<std::vector<int8_t>>& value);
    void write_array_of_int16(
      const std::string& field_name,
      const boost::optional<std::vector<int16_t>>& value);
    void write_array_of_int32(
      const std::string& field_name,
      const boost::optional<std::vector<int32_t>>& value);
    void write_array_of_int64(
      const std::string& field_name,
      const boost::optional<std::vector<int64_t>>& value);
    void write_array_of_float32(
      const std::string& field_name,
      const boost::optional<std::vector<float>>& value);
    void write_array_of_float64(
      const std::string& field_name,
      const boost::optional<std::vector<double>>& value);
    void write_array_of_string(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<std::string>>>& value);
    void write_array_of_decimal(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<big_decimal>>>& value);
    void write_array_of_time(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<local_time>>>& value);
    void write_array_of_date(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<local_date>>>& value);
    void write_array_of_timestamp(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<local_date_time>>>&
        value);
    void write_array_of_timestamp_with_timezone(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<offset_date_time>>>&
        value);
    template<typename T>
    void write_array_of_compact(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<T>>>& value);
    void write_nullable_boolean(const std::string& field_name,
                                const boost::optional<bool>& value);
    void write_nullable_int8(const std::string& field_name,
                             const boost::optional<int8_t>& value);
    void write_nullable_int16(const std::string& field_name,
                              const boost::optional<int16_t>& value);
    void write_nullable_int32(const std::string& field_name,
                              const boost::optional<int32_t>& value);
    void write_nullable_int64(const std::string& field_name,
                              const boost::optional<int64_t>& value);
    void write_nullable_float32(const std::string& field_name,
                                const boost::optional<float>& value);
    void write_nullable_float64(const std::string& field_name,
                                const boost::optional<double>& value);
    void write_array_of_nullable_boolean(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<bool>>>& value);
    void write_array_of_nullable_int8(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int8_t>>>& value);
    void write_array_of_nullable_int16(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int16_t>>>& value);
    void write_array_of_nullable_int32(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int32_t>>>& value);
    void write_array_of_nullable_int64(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<int64_t>>>& value);
    void write_array_of_nullable_float32(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<float>>>& value);
    void write_array_of_nullable_float64(
      const std::string& field_name,
      const boost::optional<std::vector<boost::optional<double>>>& value);
    void end();

private:
    size_t get_fixed_size_field_position(const std::string& field_name,
                                         enum field_kind field_kind) const;
    const field_descriptor& check_field_definition(
      const std::string& field_name,
      enum field_kind field_kind) const;
    void write_offsets(size_t data_length, const std::vector<int32_t>& offsets);

    template<typename T>
    void write_variable_size_field(const std::string& field_name,
                                   enum field_kind field_kind,
                                   const boost::optional<T>& value);

    template<typename T>
    typename std::enable_if<
      std::is_same<bool, typename std::remove_cv<T>::type>::value ||
        std::is_same<int8_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
        std::is_same<float, typename std::remove_cv<T>::type>::value ||
        std::is_same<double, typename std::remove_cv<T>::type>::value ||
        std::is_same<std::string, typename std::remove_cv<T>::type>::value ||
        std::is_same<std::vector<int8_t>, T>::value ||
        std::is_same<std::vector<int16_t>, T>::value ||
        std::is_same<std::vector<int32_t>, T>::value ||
        std::is_same<std::vector<int64_t>, T>::value ||
        std::is_same<std::vector<float>, T>::value ||
        std::is_same<std::vector<double>, T>::value ||
        std::is_same<std::vector<std::string>, T>::value,
      void>::type
    write(const T& value);

    template<typename T>
    typename std::enable_if<
      std::is_base_of<compact_serializer, hz_serializer<T>>::value,
      void>::type
    write(const T& value);

    template<typename T>
    typename std::enable_if<std::is_same<std::vector<bool>, T>::value,
                            void>::type
    write(const T& value);
    template<typename T>
    typename std::enable_if<
      std::is_same<big_decimal, typename std::remove_cv<T>::type>::value ||
        std::is_same<local_time, typename std::remove_cv<T>::type>::value ||
        std::is_same<local_date, typename std::remove_cv<T>::type>::value ||
        std::is_same<local_date_time,
                     typename std::remove_cv<T>::type>::value ||
        std::is_same<offset_date_time, typename std::remove_cv<T>::type>::value,
      void>::type
    write(const T& value);
    template<typename T>
    void write_array_of_variable_size(
      const std::string& field_name,
      enum field_kind field_kind,
      const boost::optional<std::vector<boost::optional<T>>>& value);

    void set_position(const std::string& field_name,
                      enum field_kind field_kind);
    void set_position_as_null(const std::string& field_name,
                              enum field_kind field_kind);
    compact_stream_serializer& compact_stream_serializer_;
    object_data_output& object_data_output_;
    const schema& schema_;
    size_t data_start_position;
    std::vector<int32_t> field_offsets;
};

} // namespace pimpl

namespace pimpl {
class HAZELCAST_API schema_writer
{
public:
    explicit schema_writer(std::string type_name);
    void add_field(std::string field_name, enum field_kind kind);
    schema build() &&;

private:
    std::unordered_map<std::string, field_descriptor> field_definition_map;
    std::string type_name;
};

class HAZELCAST_API compact_stream_serializer
{
public:
    compact_stream_serializer(default_schema_service&);

    template<typename T>
    T read(object_data_input& in);

    template<typename T>
    void write(const T& object, object_data_output& out);

private:
    default_schema_service& schema_service;
};

struct HAZELCAST_API field_kind_based_operations
{
    static constexpr int VARIABLE_SIZE = -1;

    static constexpr int DEFAULT_KIND_SIZE_IN_BYTES() { return VARIABLE_SIZE; }

    field_kind_based_operations();

    explicit field_kind_based_operations(
      std::function<int()> kind_size_in_byte_func);

    std::function<int()> kind_size_in_byte_func;
};

struct HAZELCAST_API field_operations
{
    static field_kind_based_operations get(enum field_kind field_kind);
};

/**
 * A very collision-resistant fingerprint method used to create automatic
 * schema ids for the Compact format.
 */
class HAZELCAST_API rabin_finger_print
{
public:
    /**
     * We use uint64_t for computation to match the behaviour of >>> operator
     * on java. We use >> instead.
     */
    static constexpr uint64_t INIT = 0xc15d213aa4d7a795L;

    static uint64_t fingerprint64(uint64_t fp, byte b);

    /**
     * FingerPrint of a little endian representation of an integer.
     */
    static uint64_t fingerprint64(uint64_t fp, int32_t v);

    static uint64_t fingerprint64(uint64_t fp, const std::string& value);

    /**
     * Calculates the fingerprint of the schema from its 'type_name' and
     * 'fields'. Fields must be sorted so it accepts fields as std::map
     */
    static int64_t fingerprint64(
      const std::string& type_name,
      std::map<std::string, field_descriptor>& fields);
};

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast
#endif // HAZELCAST_CLIENT_SERIALIZATION_IMPL_SCHEMA_H_
