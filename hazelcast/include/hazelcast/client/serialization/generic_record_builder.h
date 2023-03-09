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

#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <boost/variant.hpp>

#include "hazelcast/client/serialization/pimpl/compact/schema_writer.h"
#include "hazelcast/client/serialization/generic_record.h"
#include "hazelcast/client/big_decimal.h"
#include "hazelcast/client/local_time.h"
#include "hazelcast/client/local_date.h"
#include "hazelcast/client/local_date_time.h"
#include "hazelcast/client/offset_date_time.h"
#include "hazelcast/util/byte.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {
class compact_stream_serializer;
}
namespace generic_record {

/**
 * Concrete class for creating {@link generic_record} instances.
 * It is not thread-safe.
 * @since 5.2
 */
class HAZELCAST_API generic_record_builder
{
public:
    /**
     * Creates a Builder that will build a {@link generic_record} in
     * Compact format with the given type name and schema:
     * <pre>{@code
     * generic_record record = generic_record_builder{"typeName"}
     *                                 .set_string("name", "foo")
     *                                 .set_int32("id", 123)
     *                                 .build();
     * }</pre>
     *
     * @param typeName of the schema
     * @return generic_record_builder for Compact format
     */
    explicit generic_record_builder(std::string type_name);

    /**
     * @return a new constructed generic_record
     * @throws exception::hazelcast_serialization when the generic_record cannot
     * be built.
     */
    generic_record build();

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_boolean(std::string field_name, bool value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema/class
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_int8(std::string field_name, int8_t value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     * the schema definition or the same field is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_int16(std::string field_name, int16_t value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_int32(std::string field_name, int32_t value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_int64(std::string field_name, int64_t value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     *                  definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_float32(std::string field_name, float value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_float64(std::string field_name, double value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_boolean(std::string field_name,
                                                 boost::optional<bool> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_int8(std::string field_name,
                                              boost::optional<int8_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_int16(std::string field_name,
                                               boost::optional<int16_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_int32(std::string field_name,
                                               boost::optional<int32_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_int64(std::string field_name,
                                               boost::optional<int64_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_float32(std::string field_name,
                                                 boost::optional<float> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_nullable_float64(std::string field_name,
                                                 boost::optional<double> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_string(std::string field_name,
                                       boost::optional<std::string> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema/class
     *                  definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_string(std::string field_name,
                                       const char* cstr);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. This method allows nested
     * structures. Subclass should also be created as `generic_record`.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema/class definition or the type of the field does not
     *                                            match the one in the
     * schema/class definition or the same field is trying to be set without
     * using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_generic_record(
      std::string field_name,
      boost::optional<generic_record> value);

    /**
     * Sets a decimal which is arbitrary precision and scale floating-point
     * number.
     * <p>
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_decimal(std::string field_name,
                                        boost::optional<big_decimal> value);

    /**
     * Sets a time field consisting of hour, minute, seconds, and nanos parts.
     * <p>
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_time(std::string field_name,
                                     boost::optional<local_time> value);

    /**
     * Sets a date field consisting of year, month of the year, and day of the
     * month.
     * <p>
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_date(std::string field_name,
                                     boost::optional<local_date> value);

    /**
     * Sets a timestamp field consisting of year, month of the year, and day of
     * the month, hour, minute, seconds, nanos parts.
     * <p>
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_timestamp(
      std::string field_name,
      boost::optional<local_date_time> value);

    /**
     * Sets a timestamp with timezone field consisting of year, month of the
     * year and day of the month, offset seconds, hour, minute, seconds, nanos
     * parts.
     * <p>
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_timestamp_with_timezone(
      std::string field_name,
      boost::optional<offset_date_time> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     *                                            generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_boolean(
      std::string field_name,
      boost::optional<std::vector<bool>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     *                                            generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_boolean(std::string field_name,
                                                 std::vector<bool> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int8(
      std::string field_name,
      boost::optional<std::vector<int8_t>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int8(std::string field_name,
                                              std::vector<int8_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int16(
      std::string field_name,
      boost::optional<std::vector<int16_t>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int16(std::string field_name,
                                               std::vector<int16_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int32(
      std::string field_name,
      boost::optional<std::vector<int32_t>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int32(std::string field_name,
                                               std::vector<int32_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int64(
      std::string field_name,
      boost::optional<std::vector<int64_t>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_int64(std::string field_name,
                                               std::vector<int64_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_float32(
      std::string field_name,
      boost::optional<std::vector<float>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_float32(std::string field_name,
                                                 std::vector<float> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                         {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_float64(
      std::string field_name,
      boost::optional<std::vector<double>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                         {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_float64(std::string field_name,
                                                 std::vector<double> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_boolean(
      std::string field_name,
      boost::optional<std::vector<boost::optional<bool>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact.
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_boolean(
      std::string field_name,
      std::vector<bool> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int8(
      std::string field_name,
      boost::optional<std::vector<boost::optional<int8_t>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int8(
      std::string field_name,
      std::vector<int8_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int16(
      std::string field_name,
      boost::optional<std::vector<boost::optional<int16_t>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int16(
      std::string field_name,
      std::vector<int16_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int32(
      std::string field_name,
      boost::optional<std::vector<boost::optional<int32_t>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int32(
      std::string field_name,
      std::vector<int32_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int64(
      std::string field_name,
      boost::optional<std::vector<boost::optional<int64_t>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_int64(
      std::string field_name,
      std::vector<int64_t> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_float32(
      std::string field_name,
      boost::optional<std::vector<boost::optional<float>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_float32(
      std::string field_name,
      std::vector<float> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_float64(
      std::string field_name,
      boost::optional<std::vector<boost::optional<double>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. <p> Supported only for Compact
     * Note that, array will be copied to transform values.
     *
     * @param field_name name of the field as it is defined in its schema.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema or the type of the field does not match the one in
     *                                            the schema or the same field
     * is trying to be set without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_nullable_float64(
      std::string field_name,
      std::vector<double> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. Array items can not be null.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_string(
      std::string field_name,
      boost::optional<std::vector<boost::optional<std::string>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. Array items can not be null.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     * @see set_decimal()
     */
    generic_record_builder& set_array_of_decimal(
      std::string field_name,
      boost::optional<std::vector<boost::optional<big_decimal>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. Array items can not be null.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     * @see set_time()
     */
    generic_record_builder& set_array_of_time(
      std::string field_name,
      boost::optional<std::vector<boost::optional<local_time>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. Array items can not be null.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     * @see set_date()
     */
    generic_record_builder& set_array_of_date(
      std::string field_name,
      boost::optional<std::vector<boost::optional<local_date>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. Array items can not be null.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     * @see set_timestamp()
     */
    generic_record_builder& set_array_of_timestamp(
      std::string field_name,
      boost::optional<std::vector<boost::optional<local_date_time>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. Array items can not be null.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     * @see set_timestamp_with_timezone()
     */
    generic_record_builder& set_array_of_timestamp_with_timezone(
      std::string field_name,
      boost::optional<std::vector<boost::optional<offset_date_time>>> value);

    /**
     * It is legal to set the field again only when Builder is created with
     * {@link generic_record::new_builder_with_clone()}. Otherwise, it is
     * illegal to set to the same field twice. This method allows nested
     * structures. Subclasses should also be created as `GenericRecord`. <p>
     * Array items can not be null.
     * <p>
     * For Compact objects,
     * it is not allowed write an array containing different item
     * types or a {@link exception::hazelcast_serialization} will be thrown.
     *
     * @param field_name name of the field as it is defined in its schema
     * definition.
     * @param value     to set to generic_record
     * @return itself for chaining
     * @throws exception::hazelcast_serialization if the field name does not
     * exist in the schema definition or the type of the field does not match
     * the one in the schema definition or the same field is trying to be set
     * without using
     *                                            {@link
     * generic_record::new_builder_with_clone()}.
     */
    generic_record_builder& set_array_of_generic_record(
      std::string field_name,
      boost::optional<std::vector<boost::optional<generic_record>>> value);

private:
    enum class strategy
    {
        default_builder,
        cloner,
        schema_bounded
    };

    friend class generic_record;
    friend class pimpl::compact_stream_serializer;
    generic_record_builder(
      std::shared_ptr<pimpl::schema>); // DeserializedSchemaBoundGenericRecordBuilder
    generic_record_builder(
      std::shared_ptr<pimpl::schema>,
      std::unordered_map<std::string,
                         boost::any>); // DeserializedGenericRecordCloner

    template<typename T>
    generic_record_builder& write(std::string field_name,
                                  T value,
                                  field_kind kind)
    {
        if (already_built_) {
            BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
              boost::str(boost::format("Illegal to write after record is "
                                       "built. {field : %1%, kind : %2%}") %
                         field_name % kind) });
        }

        if (strategy_ == strategy::cloner ||
            strategy_ == strategy::schema_bounded) {
            const auto& schema =
              *boost::get<std::shared_ptr<pimpl::schema>>(writer_or_schema_);

            check_type_with_schema(schema, field_name, kind);
        }

        if (strategy_ == strategy::cloner) {
            bool inserted = overwritten_fields_.insert(field_name).second;

            if (!inserted) {
                BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
                  "Field can only be written once." });
            }
        } else if (objects_.find(field_name) != end(objects_)) {
            BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
              "Field can only be written once" });
        }

        if (strategy_ == strategy::cloner) {
            auto existing = objects_.find(field_name);

            assert(existing != end(objects_));
            existing->second = std::move(value);
        } else {
            (void)objects_.emplace(field_name, std::move(value));
        }

        if (strategy_ == strategy::default_builder) {
            pimpl::schema_writer& writer =
              boost::get<pimpl::schema_writer>(writer_or_schema_);

            writer.add_field(move(field_name), kind);
        }

        return *this;
    }

    void check_type_with_schema(const pimpl::schema&,
                                const std::string& field_name,
                                field_kind) const;

    strategy strategy_;
    bool already_built_;
    std::unordered_set<std::string> overwritten_fields_;
    std::unordered_map<std::string, boost::any> objects_;
    boost::variant<pimpl::schema_writer, std::shared_ptr<pimpl::schema>> writer_or_schema_;
};

} // namespace generic_record
} // namespace serialization
} // namespace client
} // namespace hazelcast