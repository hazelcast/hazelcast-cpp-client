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

#include <unordered_set>

#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>

#include "hazelcast/client/serialization/field_kind.h"
#include "hazelcast/client/serialization/pimpl/compact/compact_util.h"
#include "hazelcast/client/big_decimal.h"
#include "hazelcast/client/local_time.h"
#include "hazelcast/client/local_date.h"
#include "hazelcast/client/local_date_time.h"
#include "hazelcast/client/offset_date_time.h"
#include "hazelcast/util/export.h"
#include "hazelcast/util/byte.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {
class compact_stream_serializer;
}
namespace generic_record {
struct generic_record_builder;

class HAZELCAST_API generic_record
{
public:
    /**
     * Creates a {@link generic_record_builder} allows to create a new object.
     * This method is a convenience method to get a builder, without creating
     * the schema/class definition for this type. Here you can see a new object
     * is constructed from an existing generic_record with its schema
     * definition:
     *
     * <pre>{@code
     * generic_record new_record = record.new_builder()
     *      .set_string("name", "bar")
     *      .set_int32("id", 4)
     *      .build();
     * }</pre>
     * <p>
     * See {@link generic_record_builder} to create a
     * generic_record in
     * {@link generic_record_builder} to create a generic_record in
     * Compact format with a different schema.
     *
     * @return an empty generic record builder with same schema as
     * this one
     */
    generic_record_builder new_builder() const;

    /**
     * Returned {@link generic_record_builder} can be used to have exact copy
     * and also just to update a couple of fields. By default, it will copy all
     * the fields. So instead of following where only the `id` field is updated,
     * <pre>{@code
     * generic_record modified_record = record.newBuilder()
     *         .set_string("name", record.get_string("name"))
     *         .set_int64("id", 4)
     *         .set_string("surname", record.get_string("surname"))
     *         .set_int32("age", record.get_int32("age"))
     *         .build();
     * }</pre>
     * `new_builder_with_clone` used as follows:
     * <pre>{@code
     * generic_record modified_record = record.new_builder_with_clone()
     *         .set_int32("id", 4)
     *         .build();
     * }</pre>
     *
     * @return a generic record builder with same schema definition as
     * this one and populated with same values.
     */
    generic_record_builder new_builder_with_clone() const;

    /**
     * @return set of field names of this generic_record
     */
    std::unordered_set<std::string> get_field_names() const;

    /**
     * Returns the kind of the field for the given field name.
     * <p>
     * If the field with the given name does not exist,
     * {@link field_kind#NOT_AVAILABLE} is returned.
     * <p>
     * This method can be used to check the existence of a field, which can be
     * useful when the class is evolved.
     *
     * @param field_name name of the field.
     * @return kind of the field
     */
    field_kind get_field_kind(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    bool get_boolean(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    bool& get_boolean(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int8_t get_int8(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int8_t& get_int8(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int16_t get_int16(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int16_t& get_int16(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int32_t get_int32(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int32_t& get_int32(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int64_t get_int64(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    int64_t& get_int64(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    float get_float32(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    float& get_float32(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    double get_float64(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    double& get_float64(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::string>& get_string(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema/class definition or the type of the field does not
     *                                         match the one in the schema/class
     *                                         definition.
     */
    boost::optional<std::string>& get_string(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return decimal which is arbitrary precision and scale floating-point
     * number as {@link big_decimal}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<big_decimal>& get_decimal(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return decimal which is arbitrary precision and scale floating-point
     * number as {@link big_decimal}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<big_decimal>& get_decimal(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return time field consisting of hour, minute, seconds and nanos parts as
     * {@link local_time}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<local_time>& get_time(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return time field consisting of hour, minute, seconds and nanos parts as
     * {@link local_time}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<local_time>& get_time(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return date field consisting of year, month of the year and day of the
     * month as {@link local_date}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<local_date>& get_date(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return date field consisting of year, month of the year and day of the
     * month as {@link local_date}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<local_date>& get_date(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return timestamp field consisting of year, month of the year, day of the
     * month, hour, minute, seconds, nanos parts as {@link local_date_time}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<local_date_time>& get_timestamp(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return timestamp field consisting of year, month of the year, day of the
     * month, hour, minute, seconds, nanos parts as {@link local_date_time}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<local_date_time>& get_timestamp(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return timestamp with timezone field consisting of year, month of the
     * year, day of the month, offset seconds, hour, minute, seconds, nanos
     * parts as {@link offset_date_time}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<offset_date_time>& get_timestamp_with_timezone(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return timestamp with timezone field consisting of year, month of the
     * year, day of the month, offset seconds, hour, minute, seconds, nanos
     * parts as {@link offset_date_time}
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<offset_date_time>& get_timestamp_with_timezone(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<generic_record>& get_generic_record(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<generic_record>& get_generic_record(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<bool>>& get_array_of_boolean(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<bool>>& get_array_of_boolean(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<int8_t>>& get_array_of_int8(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<int8_t>>& get_array_of_int8(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<int16_t>>& get_array_of_int16(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<int16_t>>& get_array_of_int16(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<int32_t>>& get_array_of_int32(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<int32_t>>& get_array_of_int32(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<int64_t>>& get_array_of_int64(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<int64_t>>& get_array_of_int64(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<float>>& get_array_of_float32(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<float>>& get_array_of_float32(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<double>>& get_array_of_float64(
      const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<double>>& get_array_of_float64(
      const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<boost::optional<std::string>>>&
    get_array_of_string(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<boost::optional<std::string>>>&
    get_array_of_string(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema/class definition.
     * @see get_decimal()
     */
    const boost::optional<std::vector<boost::optional<big_decimal>>>&
    get_array_of_decimal(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_decimal()
     */
    boost::optional<std::vector<boost::optional<big_decimal>>>&
    get_array_of_decimal(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_time()
     */
    const boost::optional<std::vector<boost::optional<local_time>>>&
    get_array_of_time(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schem definition or the type of the field does not match the
     * one in the schema definition.
     * @see get_time()
     */
    boost::optional<std::vector<boost::optional<local_time>>>&
    get_array_of_time(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_date()
     */
    const boost::optional<std::vector<boost::optional<local_date>>>&
    get_array_of_date(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_date()
     */
    boost::optional<std::vector<boost::optional<local_date>>>&
    get_array_of_date(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_timestamp()
     */
    const boost::optional<std::vector<boost::optional<local_date_time>>>&
    get_array_of_timestamp(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_timestamp()
     */
    boost::optional<std::vector<boost::optional<local_date_time>>>&
    get_array_of_timestamp(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_timestamp_with_timezone()
     */
    const boost::optional<std::vector<boost::optional<offset_date_time>>>&
    get_array_of_timestamp_with_timezone(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     * @see get_timestamp_with_timezone()
     */
    boost::optional<std::vector<boost::optional<offset_date_time>>>&
    get_array_of_timestamp_with_timezone(const std::string& field_name);

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    const boost::optional<std::vector<boost::optional<generic_record>>>&
    get_array_of_generic_record(const std::string& field_name) const;

    /**
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition.
     */
    boost::optional<std::vector<boost::optional<generic_record>>>&
    get_array_of_generic_record(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<bool> get_nullable_boolean(
      const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<bool>& get_nullable_boolean(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<int8_t> get_nullable_int8(
      const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<int8_t>& get_nullable_int8(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<int16_t> get_nullable_int16(
      const std::string& field_name) const;

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<int16_t>& get_nullable_int16(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<int32_t> get_nullable_int32(
      const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<int32_t>& get_nullable_int32(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                             the schema.
     */
    boost::optional<int64_t> get_nullable_int64(
      const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                         the schema.
     */
    boost::optional<int64_t>& get_nullable_int64(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<float> get_nullable_float32(
      const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<float>& get_nullable_float32(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<double> get_nullable_float64(
      const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<double>& get_nullable_float64(
      const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<bool>>>&
    get_array_of_nullable_boolean(const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<bool>>>&
    get_array_of_nullable_boolean(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<int8_t>>>&
    get_array_of_nullable_int8(const std::string& field_name) const;

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<int8_t>>>&
    get_array_of_nullable_int8(const std::string& field_name);

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<int16_t>>>&
    get_array_of_nullable_int16(const std::string& field_name) const;

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<int16_t>>>&
    get_array_of_nullable_int16(const std::string& field_name);

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<int32_t>>>&
    get_array_of_nullable_int32(const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<int32_t>>>&
    get_array_of_nullable_int32(const std::string& field_name);

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<int64_t>>>&
    get_array_of_nullable_int64(const std::string& field_name) const;

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<int64_t>>>&
    get_array_of_nullable_int64(const std::string& field_name);

    /**
     * Supported only for Compact.
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<float>>>&
    get_array_of_nullable_float32(const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<float>>>&
    get_array_of_nullable_float32(const std::string& field_name);

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    const boost::optional<std::vector<boost::optional<double>>>&
    get_array_of_nullable_float64(const std::string& field_name) const;

    /**
     * Supported only for Compact
     *
     * @param field_name the name of the field
     * @return the value of the field
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema.
     */
    boost::optional<std::vector<boost::optional<double>>>&
    get_array_of_nullable_float64(const std::string& field_name);

private:
    static constexpr const char* METHOD_PREFIX_FOR_ERROR_MESSAGES = "get";

    friend class generic_record_builder;
    friend class pimpl::compact_stream_serializer;

    friend std::ostream HAZELCAST_API& operator<<(std::ostream& os,
                                                  const generic_record&);
    friend bool HAZELCAST_API operator==(const generic_record&,
                                         const generic_record&);
    friend bool HAZELCAST_API operator!=(const generic_record&,
                                         const generic_record&);

    generic_record(pimpl::schema, std::unordered_map<std::string, boost::any>);

    const pimpl::schema& get_schema() const;
    friend boost::property_tree::ptree write_generic_record(
      const generic_record&);

    template<typename... Kinds>
    field_kind check(const std::string& field_name, Kinds... kinds_ts) const
    {
        field_kind kinds[] = { kinds_ts... };
        boost::optional<pimpl::field_descriptor> desc =
          schema_.get_field(field_name);

        if (!desc) {
            BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
              boost::str(boost::format("Invalid field name : '%1%' for %2%") %
                         field_name % schema_) });
        }

        field_kind current_kind = desc->kind;
        bool valid = false;
        for (field_kind kind : kinds) {
            valid |= current_kind == kind;
        }

        if (!valid) {
            std::vector<std::string> kinds_str;
            kinds_str.reserve(sizeof...(Kinds));

            transform(std::begin(kinds),
                      std::end(kinds),
                      back_inserter(kinds_str),
                      [](field_kind kind) {
                          return boost::str(boost::format("%1%") % kind);
                      });

            BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
              boost::str(boost::format("Invalid field kind: '%1%' for %2%, "
                                       "valid field kinds : %3%, found : %4%") %
                         field_name % schema_ %
                         boost::algorithm::join(kinds_str, ",") %
                         current_kind) });
        }

        return current_kind;
    }

    template<typename T>
    const T& get(const std::string& field_name, field_kind kind) const
    {
        check(field_name, kind);
        return boost::any_cast<const T&>(objects_.at(field_name));
    }

    template<typename T>
    T& get(const std::string& field_name, field_kind kind)
    {
        check(field_name, kind);
        return boost::any_cast<T&>(objects_.at(field_name));
    }

    template<typename T>
    const T& get_non_null(const std::string& field_name,
                          field_kind primitive_field_kind,
                          field_kind nullable_field_kind,
                          const std::string& method_suffix) const
    {
        auto kind =
          check(field_name, primitive_field_kind, nullable_field_kind);

        if (kind == nullable_field_kind) {
            const auto& val_opt = boost::any_cast<const boost::optional<T>&>(
              objects_.at(field_name));

            if (!val_opt) {
                BOOST_THROW_EXCEPTION(
                  pimpl::compact_util::exception_for_unexpected_null_value(
                    field_name,
                    METHOD_PREFIX_FOR_ERROR_MESSAGES,
                    method_suffix));
            }

            return *val_opt;
        }

        return boost::any_cast<const T&>(objects_.at(field_name));
    }

    template<typename T>
    T& get_non_null(const std::string& field_name,
                    field_kind primitive_field_kind,
                    field_kind nullable_field_kind,
                    const std::string& method_suffix)
    {
        return const_cast<T&>(
          static_cast<const generic_record*>(this)->get_non_null<T>(
            field_name,
            primitive_field_kind,
            nullable_field_kind,
            method_suffix));
    }

    template<typename T>
    const boost::optional<std::vector<T>>& get_array_of_primitive(
      const std::string& field_name,
      field_kind primitive_field_kind,
      field_kind nullable_field_kind,
      const std::string& method_suffix) const
    {
        using optional_nullable_array_t =
          boost::optional<std::vector<boost::optional<T>>>;
        using optional_primitive_array_t = boost::optional<std::vector<T>>;
        using primitive_array_t = std::vector<T>;

        auto kind =
          check(field_name, primitive_field_kind, nullable_field_kind);

        if (kind == nullable_field_kind) {
            auto primitive_array_itr = adopteds_.find(field_name);

            if (primitive_array_itr != end(adopteds_)) {
                return boost::any_cast<const optional_primitive_array_t&>(
                  primitive_array_itr->second);
            }

            const auto& optional_nullable_array =
              boost::any_cast<const optional_nullable_array_t&>(
                objects_.at(field_name));

            if (!optional_nullable_array) {
                (void)adopteds_.emplace(field_name,
                                        optional_primitive_array_t{});
                return boost::any_cast<const optional_primitive_array_t&>(
                  adopteds_.at(field_name));
            }

            const auto& array_of_nullable = optional_nullable_array.value();
            optional_primitive_array_t primitive_array{ primitive_array_t(
              array_of_nullable.size()) };

            for (std::size_t i = 0; i < array_of_nullable.size(); ++i) {
                if (!array_of_nullable[i]) {
                    BOOST_THROW_EXCEPTION(
                      pimpl::compact_util::
                        exception_for_unexpected_null_value_in_array(
                          field_name,
                          METHOD_PREFIX_FOR_ERROR_MESSAGES,
                          method_suffix));
                }

                primitive_array.value()[i] = *array_of_nullable[i];
            }

            (void)adopteds_.emplace(field_name, std::move(primitive_array));
            return boost::any_cast<const optional_primitive_array_t&>(
              adopteds_.at(field_name));
        }

        return boost::any_cast<const optional_primitive_array_t&>(
          objects_.at(field_name));
    }

    template<typename T>
    boost::optional<std::vector<T>>& get_array_of_primitive(
      const std::string& field_name,
      field_kind primitive,
      field_kind nullable,
      const std::string& method_suffix)
    {
        return const_cast<boost::optional<std::vector<T>>&>(
          static_cast<const generic_record*>(this)->get_array_of_primitive<T>(
            field_name, primitive, nullable, method_suffix));
    }

    template<typename T>
    const boost::optional<std::vector<boost::optional<T>>>&
    get_array_of_nullable(const std::string& field_name,
                          field_kind primitive,
                          field_kind nullable,
                          const std::string& method_suffix) const
    {
        using optional_nullable_array_t =
          boost::optional<std::vector<boost::optional<T>>>;
        using optional_primitive_array_t = boost::optional<std::vector<T>>;
        using optional_array_t = std::vector<boost::optional<T>>;

        auto kind = check(field_name, primitive, nullable);

        if (kind == primitive) {
            auto primitive_array_itr = adopteds_.find(field_name);

            if (primitive_array_itr != end(adopteds_)) {
                return boost::any_cast<const optional_nullable_array_t&>(
                  primitive_array_itr->second);
            }

            const auto& optional_primitive_array =
              boost::any_cast<const optional_primitive_array_t&>(
                objects_.at(field_name));

            if (!optional_primitive_array) {
                (void)adopteds_.emplace(field_name,
                                        optional_nullable_array_t{});
                return boost::any_cast<const optional_nullable_array_t&>(
                  adopteds_.at(field_name));
            }

            const auto& array_of_primitive = optional_primitive_array.value();
            optional_nullable_array_t array_of_nullable{ optional_array_t(
              array_of_primitive.size()) };

            for (std::size_t i = 0; i < array_of_primitive.size(); ++i) {
                array_of_nullable.value()[i] = array_of_primitive[i];
            }

            (void)adopteds_.emplace(field_name, std::move(array_of_nullable));
            return boost::any_cast<const optional_nullable_array_t&>(
              adopteds_.at(field_name));
        }

        return boost::any_cast<const optional_nullable_array_t&>(
          objects_.at(field_name));
    }

    template<typename T>
    boost::optional<std::vector<boost::optional<T>>>& get_array_of_nullable(
      const std::string& field_name,
      field_kind primitive,
      field_kind nullable,
      const std::string& method_suffix)
    {
        return const_cast<boost::optional<std::vector<boost::optional<T>>>&>(
          static_cast<const generic_record*>(this)->get_array_of_nullable<T>(
            field_name, primitive, nullable, method_suffix));
    }

    pimpl::schema schema_;
    std::unordered_map<std::string, boost::any> objects_;

    /**
     * Exceptional case for C++
     * generic_record supports returning array of nullable primitives as array
     * of primitives and array of primitives as array of nullable primitives e.g
     * get_array_of_boolean(), get_array_of_nullable_boolean() Getter methods
     * for array is returned by reference, so if the adoption happens then
     * adopted objects must live somewhere, 'adopteds_' hosts those adoptions.
     */
    mutable std::unordered_map<std::string, boost::any> adopteds_;
};

} // namespace generic_record
} // namespace serialization
} // namespace client
} // namespace hazelcast