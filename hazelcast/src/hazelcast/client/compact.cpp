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

#include <utility>
#include <algorithm>
#include <thread>
#include <chrono>
#include <functional>

#include <boost/property_tree/json_parser.hpp>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/field_kind.h"
#include "hazelcast/client/serialization/pimpl/compact/schema.h"
#include "hazelcast/client/serialization/generic_record_builder.h"
#include "hazelcast/client/serialization/generic_record.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/cluster.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/client_properties.h"

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {

field_descriptor::field_descriptor(field_kind k, int32_t i, int32_t o, int8_t b)
  : kind{ k }
  , index{ i }
  , offset{ o }
  , bit_offset{ b }
{
}

bool
operator==(const field_descriptor& x, const field_descriptor& y)
{
    return x.kind == y.kind;
}

std::ostream&
operator<<(std::ostream& os, const field_descriptor& fd)
{
    return os << "FieldDescriptor{"
              << "kind=" << fd.kind << ", index=" << fd.index
              << ", offset=" << fd.offset << ", bitOffset=" << fd.bit_offset
              << '}';
}

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace serialization {
namespace generic_record {

generic_record_builder&
generic_record_builder::set_string(std::string field_name, const char* cstr)
{
    return set_string(field_name,
                      cstr ? boost::optional<std::string>{ std::string{ cstr } }
                           : boost::none);
}

generic_record_builder::generic_record_builder(std::string type_name)
  : strategy_{ strategy::default_builder }
  , already_built_{ false }
  , writer_or_schema_{ pimpl::schema_writer{ move(type_name) } }
{
}

generic_record_builder::generic_record_builder(std::shared_ptr<pimpl::schema> record_schema)
  : strategy_{ strategy::schema_bounded }
  , already_built_{ false }
  , writer_or_schema_{ std::move(record_schema) }
{
}

generic_record_builder::generic_record_builder(
  std::shared_ptr<pimpl::schema> boundary,
  std::unordered_map<std::string, boost::any> objects)
  : strategy_{ strategy::cloner }
  , already_built_{ false }
  , objects_{ std::move(objects) }
  , writer_or_schema_{ std::move(boundary) }
{
}

void
generic_record_builder::check_type_with_schema(const pimpl::schema& schema,
                                               const std::string& field_name,
                                               field_kind kind) const
{
    boost::optional<pimpl::field_descriptor> fd = schema.get_field(field_name);

    if (!fd) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
          boost::str(boost::format("Invalid field name: '%1%' for %2%") %
                     field_name % schema) });
    }

    if (fd->kind != kind) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{ boost::str(
          boost::format(
            "Invalid field kind: '%1%' for %2%, expected : %3%, given : %4%") %
          field_name % schema % fd->kind % kind) });
    }
}

generic_record
generic_record_builder::build()
{
    if (strategy_ == strategy::default_builder) {
        pimpl::schema_writer& writer =
          boost::get<pimpl::schema_writer>(writer_or_schema_);

        already_built_ = true;
        return generic_record{ std::make_shared<pimpl::schema>(std::move(writer).build()), std::move(objects_) };
    } else {
        std::shared_ptr<pimpl::schema> schema = boost::get<std::shared_ptr<pimpl::schema>>(writer_or_schema_);

        if (strategy_ == strategy::schema_bounded) {
            const auto& fields = schema->fields();

            for (const auto& p : fields) {
                const std::string& field_name = p.first;

                if (objects_.find(field_name) == end(objects_)) {
                    BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
                      "Found an unset field " + field_name +
                      ". All the fields must be set before build" });
                }
            }
        }

        already_built_ = true;
        return generic_record{ move(schema), std::move(objects_) };
    }
}

generic_record_builder&
generic_record_builder::set_boolean(std::string field_name, bool value)
{
    return write(move(field_name), value, field_kind::BOOLEAN);
}

generic_record_builder&
generic_record_builder::set_int8(std::string field_name, int8_t value)
{
    return write(move(field_name), value, field_kind::INT8);
}

generic_record_builder&
generic_record_builder::set_int16(std::string field_name, int16_t value)
{
    return write(move(field_name), value, field_kind::INT16);
}

generic_record_builder&
generic_record_builder::set_int32(std::string field_name, int32_t value)
{
    return write(move(field_name), value, field_kind::INT32);
}

generic_record_builder&
generic_record_builder::set_int64(std::string field_name, int64_t value)
{
    return write(move(field_name), value, field_kind::INT64);
}

generic_record_builder&
generic_record_builder::set_float32(std::string field_name, float value)
{
    return write(move(field_name), value, field_kind::FLOAT32);
}

generic_record_builder&
generic_record_builder::set_float64(std::string field_name, double value)
{
    return write(move(field_name), value, field_kind::FLOAT64);
}

generic_record_builder&
generic_record_builder::set_nullable_boolean(std::string field_name,
                                             boost::optional<bool> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_BOOLEAN);
}

generic_record_builder&
generic_record_builder::set_nullable_int8(std::string field_name,
                                          boost::optional<int8_t> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_INT8);
}

generic_record_builder&
generic_record_builder::set_nullable_int16(std::string field_name,
                                           boost::optional<int16_t> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_INT16);
}

generic_record_builder&
generic_record_builder::set_nullable_int32(std::string field_name,
                                           boost::optional<int32_t> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_INT32);
}

generic_record_builder&
generic_record_builder::set_nullable_int64(std::string field_name,
                                           boost::optional<int64_t> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_INT64);
}

generic_record_builder&
generic_record_builder::set_nullable_float32(std::string field_name,
                                             boost::optional<float> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_FLOAT32);
}

generic_record_builder&
generic_record_builder::set_nullable_float64(std::string field_name,
                                             boost::optional<double> value)
{
    return write(move(field_name), value, field_kind::NULLABLE_FLOAT64);
}

generic_record_builder&
generic_record_builder::set_generic_record(
  std::string field_name,
  boost::optional<generic_record> value)
{
    return write(move(field_name), std::move(value), field_kind::COMPACT);
}

generic_record_builder&
generic_record_builder::set_decimal(std::string field_name,
                                    boost::optional<big_decimal> value)
{
    return write(move(field_name), value, field_kind::DECIMAL);
}

generic_record_builder&
generic_record_builder::set_time(std::string field_name,
                                 boost::optional<local_time> value)
{
    return write(move(field_name), value, field_kind::TIME);
}

generic_record_builder&
generic_record_builder::set_date(std::string field_name,
                                 boost::optional<local_date> value)
{
    return write(move(field_name), value, field_kind::DATE);
}

generic_record_builder&
generic_record_builder::set_timestamp(std::string field_name,
                                      boost::optional<local_date_time> value)
{
    return write(move(field_name), value, field_kind::TIMESTAMP);
}

generic_record_builder&
generic_record_builder::set_timestamp_with_timezone(
  std::string field_name,
  boost::optional<offset_date_time> value)
{
    return write(move(field_name), value, field_kind::TIMESTAMP_WITH_TIMEZONE);
}

generic_record_builder&
generic_record_builder::set_string(std::string field_name,
                                   boost::optional<std::string> value)
{
    return write(move(field_name), std::move(value), field_kind::STRING);
}

generic_record_builder&
generic_record_builder::set_array_of_boolean(
  std::string field_name,
  boost::optional<std::vector<bool>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_BOOLEAN);
}

generic_record_builder&
generic_record_builder::set_array_of_boolean(std::string field_name,
                                             std::vector<bool> value)
{
    return set_array_of_boolean(
      move(field_name), boost::optional<std::vector<bool>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_boolean(std::string field_name,
	std::initializer_list<bool> value)
{
	return set_array_of_boolean(
		move(field_name), std::vector<bool>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_int8(
  std::string field_name,
  boost::optional<std::vector<int8_t>> value)
{
    return write(move(field_name), std::move(value), field_kind::ARRAY_OF_INT8);
}

generic_record_builder&
generic_record_builder::set_array_of_int8(std::string field_name,
                                          std::vector<int8_t> value)
{
    return set_array_of_int8(move(field_name),
                             boost::optional<std::vector<int8_t>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_int8(std::string field_name,
	std::initializer_list<int8_t> value)
{
	return set_array_of_int8(move(field_name),
		std::vector<int8_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_int16(
  std::string field_name,
  boost::optional<std::vector<int16_t>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_INT16);
}

generic_record_builder&
generic_record_builder::set_array_of_int16(std::string field_name,
                                           std::vector<int16_t> value)
{
    return set_array_of_int16(
      move(field_name), boost::optional<std::vector<int16_t>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_int16(std::string field_name,
	std::initializer_list<int16_t> value)
{
	return set_array_of_int16(
		move(field_name), std::vector<int16_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_int32(
  std::string field_name,
  boost::optional<std::vector<int32_t>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_INT32);
}

generic_record_builder&
generic_record_builder::set_array_of_int32(std::string field_name,
                                           std::vector<int32_t> value)
{
    return set_array_of_int32(
      move(field_name), boost::optional<std::vector<int32_t>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_int32(std::string field_name,
	std::initializer_list<int32_t> value)
{
	return set_array_of_int32(
		move(field_name), std::vector<int32_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_int64(
  std::string field_name,
  boost::optional<std::vector<int64_t>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_INT64);
}

generic_record_builder&
generic_record_builder::set_array_of_int64(std::string field_name,
                                           std::vector<int64_t> value)
{
    return set_array_of_int64(
      move(field_name), boost::optional<std::vector<int64_t>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_int64(std::string field_name,
	std::initializer_list<int64_t> value)
{
	return set_array_of_int64(
		move(field_name), std::vector<int64_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_float32(
  std::string field_name,
  boost::optional<std::vector<float>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_FLOAT32);
}

generic_record_builder&
generic_record_builder::set_array_of_float32(std::string field_name,
                                             std::vector<float> value)
{
    return set_array_of_float32(
      move(field_name), boost::optional<std::vector<float>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_float32(std::string field_name,
	std::initializer_list<float> value)
{
	return set_array_of_float32(
		move(field_name), std::vector<float>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_float64(
  std::string field_name,
  boost::optional<std::vector<double>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_FLOAT64);
}

generic_record_builder&
generic_record_builder::set_array_of_float64(std::string field_name,
                                             std::vector<double> value)
{
    return set_array_of_float64(
      move(field_name), boost::optional<std::vector<double>>(move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_float64(std::string field_name,
	std::initializer_list<double> value)
{
	return set_array_of_float64(
		move(field_name), std::vector<double>(value));
}

template<typename T>
std::vector<boost::optional<T>>
to_nullable(std::vector<T> value)
{
	std::vector<boost::optional<T>> value_opt;

	value_opt.reserve(value.size());

	transform(begin(value), end(value), back_inserter(value_opt), [](T value) {
		return boost::optional<T>{ value };
		});

	return value_opt;
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_boolean(
  std::string field_name,
  boost::optional<std::vector<boost::optional<bool>>> value)
{
    return write(move(field_name),
                 std::move(value),
                 field_kind::ARRAY_OF_NULLABLE_BOOLEAN);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_boolean(std::string field_name,
                                                      std::vector<bool> value)
{
    return set_array_of_nullable_boolean(move(field_name),
                                         to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_boolean(std::string field_name,
	std::initializer_list<bool> value)
{
	return set_array_of_nullable_boolean(move(field_name),
		std::vector<bool>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int8(
  std::string field_name,
  boost::optional<std::vector<boost::optional<int8_t>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_NULLABLE_INT8);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int8(std::string field_name,
                                                   std::vector<int8_t> value)
{
    return set_array_of_nullable_int8(move(field_name),
                                      to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int8(std::string field_name,
	std::initializer_list<int8_t> value)
{
	return set_array_of_nullable_int8(move(field_name),
		std::vector<int8_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int16(
  std::string field_name,
  boost::optional<std::vector<boost::optional<int16_t>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_NULLABLE_INT16);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int16(std::string field_name,
                                                    std::vector<int16_t> value)
{
    return set_array_of_nullable_int16(move(field_name),
                                       to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int16(std::string field_name,
	std::initializer_list<int16_t> value)
{
	return set_array_of_nullable_int16(move(field_name),
		std::vector<int16_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int32(
  std::string field_name,
  boost::optional<std::vector<boost::optional<int32_t>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_NULLABLE_INT32);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int32(std::string field_name,
                                                    std::vector<int32_t> value)
{
    return set_array_of_nullable_int32(move(field_name),
                                       to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int32(std::string field_name,
	std::initializer_list<int32_t> value)
{
	return set_array_of_nullable_int32(move(field_name),
		std::vector<int32_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int64(
  std::string field_name,
  boost::optional<std::vector<boost::optional<int64_t>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_NULLABLE_INT64);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int64(std::string field_name,
                                                    std::vector<int64_t> value)
{
    return set_array_of_nullable_int64(move(field_name),
                                       to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_int64(std::string field_name,
	std::initializer_list<int64_t> value)
{
	return set_array_of_nullable_int64(move(field_name),
		std::vector<int64_t>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_float32(
  std::string field_name,
  boost::optional<std::vector<boost::optional<float>>> value)
{
    return write(move(field_name),
                 std::move(value),
                 field_kind::ARRAY_OF_NULLABLE_FLOAT32);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_float32(std::string field_name,
                                                      std::vector<float> value)
{
    return set_array_of_nullable_float32(move(field_name),
                                         to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_float32(std::string field_name,
	std::initializer_list<float> value)
{
	return set_array_of_nullable_float32(move(field_name),
		std::vector<float>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_float64(
  std::string field_name,
  boost::optional<std::vector<boost::optional<double>>> value)
{
    return write(move(field_name),
                 std::move(value),
                 field_kind::ARRAY_OF_NULLABLE_FLOAT64);
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_float64(std::string field_name,
                                                      std::vector<double> value)
{
    return set_array_of_nullable_float64(move(field_name),
                                         to_nullable(std::move(value)));
}

generic_record_builder&
generic_record_builder::set_array_of_nullable_float64(std::string field_name,
	std::initializer_list<double> value)
{
	return set_array_of_nullable_float64(move(field_name),
		std::vector<double>(value));
}

generic_record_builder&
generic_record_builder::set_array_of_string(
  std::string field_name,
  boost::optional<std::vector<boost::optional<std::string>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_STRING);
}

generic_record_builder&
generic_record_builder::set_array_of_decimal(
  std::string field_name,
  boost::optional<std::vector<boost::optional<big_decimal>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_DECIMAL);
}

generic_record_builder&
generic_record_builder::set_array_of_time(
  std::string field_name,
  boost::optional<std::vector<boost::optional<local_time>>> value)
{
    return write(move(field_name), std::move(value), field_kind::ARRAY_OF_TIME);
}

generic_record_builder&
generic_record_builder::set_array_of_date(
  std::string field_name,
  boost::optional<std::vector<boost::optional<local_date>>> value)
{
    return write(move(field_name), std::move(value), field_kind::ARRAY_OF_DATE);
}

generic_record_builder&
generic_record_builder::set_array_of_timestamp(
  std::string field_name,
  boost::optional<std::vector<boost::optional<local_date_time>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_TIMESTAMP);
}

generic_record_builder&
generic_record_builder::set_array_of_timestamp_with_timezone(
  std::string field_name,
  boost::optional<std::vector<boost::optional<offset_date_time>>> value)
{
    return write(move(field_name),
                 std::move(value),
                 field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE);
}

generic_record_builder&
generic_record_builder::set_array_of_generic_record(
  std::string field_name,
  boost::optional<std::vector<boost::optional<generic_record>>> value)
{
    return write(
      move(field_name), std::move(value), field_kind::ARRAY_OF_COMPACT);
}

generic_record::generic_record(
  std::shared_ptr<pimpl::schema> s,
  std::unordered_map<std::string, boost::any> objects)
  : schema_{ std::move(s) }
  , objects_{ std::move(objects) }
{
}

const pimpl::schema&
generic_record::get_schema() const
{
    return *schema_;
}

generic_record_builder
generic_record::new_builder() const
{
    return generic_record_builder{ schema_ };
}

generic_record_builder
generic_record::new_builder_with_clone() const
{
    return generic_record_builder{ schema_, objects_ };
}

std::unordered_set<std::string>
generic_record::get_field_names() const
{
    std::unordered_set<std::string> field_names;

    const auto& fields = schema_->fields();

    transform(begin(fields),
              end(fields),
              inserter(field_names, end(field_names)),
              [](const std::pair<std::string, pimpl::field_descriptor>& p) {
                  return p.first;
              });

    return field_names;
}

field_kind
generic_record::get_field_kind(const std::string& field_name) const
{
    auto descriptor = schema_->get_field(field_name);

    if (!descriptor) {
        return field_kind::NOT_AVAILABLE;
    }

    return descriptor->kind;
}

bool
generic_record::get_boolean(const std::string& field_name) const
{
    return get_non_null<bool>(
      field_name, field_kind::BOOLEAN, field_kind::NULLABLE_BOOLEAN, "Boolean");
}

bool&
generic_record::get_boolean(const std::string& field_name)
{
    return get_non_null<bool>(
      field_name, field_kind::BOOLEAN, field_kind::NULLABLE_BOOLEAN, "Boolean");
}

int8_t
generic_record::get_int8(const std::string& field_name) const
{
    return get_non_null<int8_t>(
      field_name, field_kind::INT8, field_kind::NULLABLE_INT8, "Int8");
}

int8_t&
generic_record::get_int8(const std::string& field_name)
{
    return get_non_null<int8_t>(
      field_name, field_kind::INT8, field_kind::NULLABLE_INT8, "Int8");
}

int16_t
generic_record::get_int16(const std::string& field_name) const
{
    return get_non_null<int16_t>(
      field_name, field_kind::INT16, field_kind::NULLABLE_INT16, "Int16");
}

int16_t&
generic_record::get_int16(const std::string& field_name)
{
    return get_non_null<int16_t>(
      field_name, field_kind::INT16, field_kind::NULLABLE_INT16, "Int16");
}

int32_t
generic_record::get_int32(const std::string& field_name) const
{
    return get_non_null<int32_t>(
      field_name, field_kind::INT32, field_kind::NULLABLE_INT32, "Int32");
}

int32_t&
generic_record::get_int32(const std::string& field_name)
{
    return get_non_null<int32_t>(
      field_name, field_kind::INT32, field_kind::NULLABLE_INT32, "Int32");
}

int64_t
generic_record::get_int64(const std::string& field_name) const
{
    return get_non_null<int64_t>(
      field_name, field_kind::INT64, field_kind::NULLABLE_INT64, "Int64");
}

int64_t&
generic_record::get_int64(const std::string& field_name)
{
    return get_non_null<int64_t>(
      field_name, field_kind::INT64, field_kind::NULLABLE_INT64, "Int64");
}

float
generic_record::get_float32(const std::string& field_name) const
{
    return get_non_null<float>(
      field_name, field_kind::FLOAT32, field_kind::NULLABLE_FLOAT32, "Float32");
}

float&
generic_record::get_float32(const std::string& field_name)
{
    return get_non_null<float>(
      field_name, field_kind::FLOAT32, field_kind::NULLABLE_FLOAT32, "Float32");
}

double
generic_record::get_float64(const std::string& field_name) const
{
    return get_non_null<double>(
      field_name, field_kind::FLOAT64, field_kind::NULLABLE_FLOAT64, "Float64");
}

double&
generic_record::get_float64(const std::string& field_name)
{
    return get_non_null<double>(
      field_name, field_kind::FLOAT64, field_kind::NULLABLE_FLOAT64, "Float64");
}

boost::optional<bool>
generic_record::get_nullable_boolean(const std::string& field_name) const
{
    return get<boost::optional<bool>>(field_name, field_kind::NULLABLE_BOOLEAN);
}

boost::optional<bool>&
generic_record::get_nullable_boolean(const std::string& field_name)
{
    return get<boost::optional<bool>>(field_name, field_kind::NULLABLE_BOOLEAN);
}

boost::optional<int8_t>
generic_record::get_nullable_int8(const std::string& field_name) const
{
    return get<boost::optional<int8_t>>(field_name, field_kind::NULLABLE_INT8);
}

boost::optional<int8_t>&
generic_record::get_nullable_int8(const std::string& field_name)
{
    return get<boost::optional<int8_t>>(field_name, field_kind::NULLABLE_INT8);
}

boost::optional<int16_t>
generic_record::get_nullable_int16(const std::string& field_name) const
{
    return get<boost::optional<int16_t>>(field_name,
                                         field_kind::NULLABLE_INT16);
}

boost::optional<int16_t>&
generic_record::get_nullable_int16(const std::string& field_name)
{
    return get<boost::optional<int16_t>>(field_name,
                                         field_kind::NULLABLE_INT16);
}

boost::optional<int32_t>
generic_record::get_nullable_int32(const std::string& field_name) const
{
    return get<boost::optional<int32_t>>(field_name,
                                         field_kind::NULLABLE_INT32);
}

boost::optional<int32_t>&
generic_record::get_nullable_int32(const std::string& field_name)
{
    return get<boost::optional<int32_t>>(field_name,
                                         field_kind::NULLABLE_INT32);
}

boost::optional<int64_t>
generic_record::get_nullable_int64(const std::string& field_name) const
{
    return get<boost::optional<int64_t>>(field_name,
                                         field_kind::NULLABLE_INT64);
}

boost::optional<int64_t>&
generic_record::get_nullable_int64(const std::string& field_name)
{
    return get<boost::optional<int64_t>>(field_name,
                                         field_kind::NULLABLE_INT64);
}

boost::optional<float>
generic_record::get_nullable_float32(const std::string& field_name) const
{
    return get<boost::optional<float>>(field_name,
                                       field_kind::NULLABLE_FLOAT32);
}

boost::optional<float>&
generic_record::get_nullable_float32(const std::string& field_name)
{
    return get<boost::optional<float>>(field_name,
                                       field_kind::NULLABLE_FLOAT32);
}

boost::optional<double>
generic_record::get_nullable_float64(const std::string& field_name) const
{
    return get<boost::optional<double>>(field_name,
                                        field_kind::NULLABLE_FLOAT64);
}

boost::optional<double>&
generic_record::get_nullable_float64(const std::string& field_name)
{
    return get<boost::optional<double>>(field_name,
                                        field_kind::NULLABLE_FLOAT64);
}

const boost::optional<std::string>&
generic_record::get_string(const std::string& field_name) const
{
    return get<boost::optional<std::string>>(field_name, field_kind::STRING);
}

boost::optional<std::string>&
generic_record::get_string(const std::string& field_name)
{
    return get<boost::optional<std::string>>(field_name, field_kind::STRING);
}

const boost::optional<generic_record>&
generic_record::get_generic_record(const std::string& field_name) const
{
    return get<boost::optional<generic_record>>(field_name,
                                                field_kind::COMPACT);
}

boost::optional<generic_record>&
generic_record::get_generic_record(const std::string& field_name)
{
    return get<boost::optional<generic_record>>(field_name,
                                                field_kind::COMPACT);
}

const boost::optional<big_decimal>&
generic_record::get_decimal(const std::string& field_name) const
{
    return get<boost::optional<big_decimal>>(field_name, field_kind::DECIMAL);
}

boost::optional<big_decimal>&
generic_record::get_decimal(const std::string& field_name)
{
    return get<boost::optional<big_decimal>>(field_name, field_kind::DECIMAL);
}

const boost::optional<local_time>&
generic_record::get_time(const std::string& field_name) const
{
    return get<boost::optional<local_time>>(field_name, field_kind::TIME);
}

boost::optional<local_time>&
generic_record::get_time(const std::string& field_name)
{
    return get<boost::optional<local_time>>(field_name, field_kind::TIME);
}

const boost::optional<local_date>&
generic_record::get_date(const std::string& field_name) const
{
    return get<boost::optional<local_date>>(field_name, field_kind::DATE);
}

boost::optional<local_date>&
generic_record::get_date(const std::string& field_name)
{
    return get<boost::optional<local_date>>(field_name, field_kind::DATE);
}

const boost::optional<local_date_time>&
generic_record::get_timestamp(const std::string& field_name) const
{
    return get<boost::optional<local_date_time>>(field_name,
                                                 field_kind::TIMESTAMP);
}

boost::optional<local_date_time>&
generic_record::get_timestamp(const std::string& field_name)
{
    return get<boost::optional<local_date_time>>(field_name,
                                                 field_kind::TIMESTAMP);
}

const boost::optional<offset_date_time>&
generic_record::get_timestamp_with_timezone(const std::string& field_name) const
{
    return get<boost::optional<offset_date_time>>(
      field_name, field_kind::TIMESTAMP_WITH_TIMEZONE);
}

boost::optional<offset_date_time>&
generic_record::get_timestamp_with_timezone(const std::string& field_name)
{
    return get<boost::optional<offset_date_time>>(
      field_name, field_kind::TIMESTAMP_WITH_TIMEZONE);
}

const boost::optional<std::vector<bool>>&
generic_record::get_array_of_boolean(const std::string& field_name) const
{
    return get_array_of_primitive<bool>(field_name,
                                        field_kind::ARRAY_OF_BOOLEAN,
                                        field_kind::ARRAY_OF_NULLABLE_BOOLEAN,
                                        "boolean");
}

boost::optional<std::vector<bool>>&
generic_record::get_array_of_boolean(const std::string& field_name)
{
    return get_array_of_primitive<bool>(field_name,
                                        field_kind::ARRAY_OF_BOOLEAN,
                                        field_kind::ARRAY_OF_NULLABLE_BOOLEAN,
                                        "boolean");
}

const boost::optional<std::vector<int8_t>>&
generic_record::get_array_of_int8(const std::string& field_name) const
{
    return get_array_of_primitive<int8_t>(field_name,
                                          field_kind::ARRAY_OF_INT8,
                                          field_kind::ARRAY_OF_NULLABLE_INT8,
                                          "int8");
}

boost::optional<std::vector<int8_t>>&
generic_record::get_array_of_int8(const std::string& field_name)
{
    return get_array_of_primitive<int8_t>(field_name,
                                          field_kind::ARRAY_OF_INT8,
                                          field_kind::ARRAY_OF_NULLABLE_INT8,
                                          "int8");
}

const boost::optional<std::vector<int16_t>>&
generic_record::get_array_of_int16(const std::string& field_name) const
{
    return get_array_of_primitive<int16_t>(field_name,
                                           field_kind::ARRAY_OF_INT16,
                                           field_kind::ARRAY_OF_NULLABLE_INT16,
                                           "int16");
}

boost::optional<std::vector<int16_t>>&
generic_record::get_array_of_int16(const std::string& field_name)
{
    return get_array_of_primitive<int16_t>(field_name,
                                           field_kind::ARRAY_OF_INT16,
                                           field_kind::ARRAY_OF_NULLABLE_INT16,
                                           "int16");
}

const boost::optional<std::vector<int32_t>>&
generic_record::get_array_of_int32(const std::string& field_name) const
{
    return get_array_of_primitive<int32_t>(field_name,
                                           field_kind::ARRAY_OF_INT32,
                                           field_kind::ARRAY_OF_NULLABLE_INT32,
                                           "int32");
}

boost::optional<std::vector<int32_t>>&
generic_record::get_array_of_int32(const std::string& field_name)
{
    return get_array_of_primitive<int32_t>(field_name,
                                           field_kind::ARRAY_OF_INT32,
                                           field_kind::ARRAY_OF_NULLABLE_INT32,
                                           "int32");
}

const boost::optional<std::vector<int64_t>>&
generic_record::get_array_of_int64(const std::string& field_name) const
{
    return get_array_of_primitive<int64_t>(field_name,
                                           field_kind::ARRAY_OF_INT64,
                                           field_kind::ARRAY_OF_NULLABLE_INT64,
                                           "int64");
}

boost::optional<std::vector<int64_t>>&
generic_record::get_array_of_int64(const std::string& field_name)
{
    return get_array_of_primitive<int64_t>(field_name,
                                           field_kind::ARRAY_OF_INT64,
                                           field_kind::ARRAY_OF_NULLABLE_INT64,
                                           "int64");
}

const boost::optional<std::vector<float>>&
generic_record::get_array_of_float32(const std::string& field_name) const
{
    return get_array_of_primitive<float>(field_name,
                                         field_kind::ARRAY_OF_FLOAT32,
                                         field_kind::ARRAY_OF_NULLABLE_FLOAT32,
                                         "float32");
}

boost::optional<std::vector<float>>&
generic_record::get_array_of_float32(const std::string& field_name)
{
    return get_array_of_primitive<float>(field_name,
                                         field_kind::ARRAY_OF_FLOAT32,
                                         field_kind::ARRAY_OF_NULLABLE_FLOAT32,
                                         "float32");
}

const boost::optional<std::vector<double>>&
generic_record::get_array_of_float64(const std::string& field_name) const
{
    return get_array_of_primitive<double>(field_name,
                                          field_kind::ARRAY_OF_FLOAT64,
                                          field_kind::ARRAY_OF_NULLABLE_FLOAT64,
                                          "float64");
}

boost::optional<std::vector<double>>&
generic_record::get_array_of_float64(const std::string& field_name)
{
    return get_array_of_primitive<double>(field_name,
                                          field_kind::ARRAY_OF_FLOAT64,
                                          field_kind::ARRAY_OF_NULLABLE_FLOAT64,
                                          "float64");
}

const boost::optional<std::vector<boost::optional<bool>>>&
generic_record::get_array_of_nullable_boolean(
  const std::string& field_name) const
{
    return get_array_of_nullable<bool>(field_name,
                                       field_kind::ARRAY_OF_BOOLEAN,
                                       field_kind::ARRAY_OF_NULLABLE_BOOLEAN,
                                       "boolean");
}

boost::optional<std::vector<boost::optional<bool>>>&
generic_record::get_array_of_nullable_boolean(const std::string& field_name)
{
    return get_array_of_nullable<bool>(field_name,
                                       field_kind::ARRAY_OF_BOOLEAN,
                                       field_kind::ARRAY_OF_NULLABLE_BOOLEAN,
                                       "boolean");
}

const boost::optional<std::vector<boost::optional<int8_t>>>&
generic_record::get_array_of_nullable_int8(const std::string& field_name) const
{
    return get_array_of_nullable<int8_t>(field_name,
                                         field_kind::ARRAY_OF_INT8,
                                         field_kind::ARRAY_OF_NULLABLE_INT8,
                                         "int8");
}

boost::optional<std::vector<boost::optional<int8_t>>>&
generic_record::get_array_of_nullable_int8(const std::string& field_name)
{
    return get_array_of_nullable<int8_t>(field_name,
                                         field_kind::ARRAY_OF_INT8,
                                         field_kind::ARRAY_OF_NULLABLE_INT8,
                                         "int8");
}

const boost::optional<std::vector<boost::optional<int16_t>>>&
generic_record::get_array_of_nullable_int16(const std::string& field_name) const
{
    return get_array_of_nullable<int16_t>(field_name,
                                          field_kind::ARRAY_OF_INT16,
                                          field_kind::ARRAY_OF_NULLABLE_INT16,
                                          "int16");
}

boost::optional<std::vector<boost::optional<int16_t>>>&
generic_record::get_array_of_nullable_int16(const std::string& field_name)
{
    return get_array_of_nullable<int16_t>(field_name,
                                          field_kind::ARRAY_OF_INT16,
                                          field_kind::ARRAY_OF_NULLABLE_INT16,
                                          "int16");
}

const boost::optional<std::vector<boost::optional<int32_t>>>&
generic_record::get_array_of_nullable_int32(const std::string& field_name) const
{
    return get_array_of_nullable<int32_t>(field_name,
                                          field_kind::ARRAY_OF_INT32,
                                          field_kind::ARRAY_OF_NULLABLE_INT32,
                                          "int32");
}

boost::optional<std::vector<boost::optional<int32_t>>>&
generic_record::get_array_of_nullable_int32(const std::string& field_name)
{
    return get_array_of_nullable<int32_t>(field_name,
                                          field_kind::ARRAY_OF_INT32,
                                          field_kind::ARRAY_OF_NULLABLE_INT32,
                                          "int32");
}

const boost::optional<std::vector<boost::optional<int64_t>>>&
generic_record::get_array_of_nullable_int64(const std::string& field_name) const
{
    return get_array_of_nullable<int64_t>(field_name,
                                          field_kind::ARRAY_OF_INT64,
                                          field_kind::ARRAY_OF_NULLABLE_INT64,
                                          "int64");
}

boost::optional<std::vector<boost::optional<int64_t>>>&
generic_record::get_array_of_nullable_int64(const std::string& field_name)
{
    return get_array_of_nullable<int64_t>(field_name,
                                          field_kind::ARRAY_OF_INT64,
                                          field_kind::ARRAY_OF_NULLABLE_INT64,
                                          "int64");
}

const boost::optional<std::vector<boost::optional<float>>>&
generic_record::get_array_of_nullable_float32(
  const std::string& field_name) const
{
    return get_array_of_nullable<float>(field_name,
                                        field_kind::ARRAY_OF_FLOAT32,
                                        field_kind::ARRAY_OF_NULLABLE_FLOAT32,
                                        "float32");
}

boost::optional<std::vector<boost::optional<float>>>&
generic_record::get_array_of_nullable_float32(const std::string& field_name)
{
    return get_array_of_nullable<float>(field_name,
                                        field_kind::ARRAY_OF_FLOAT32,
                                        field_kind::ARRAY_OF_NULLABLE_FLOAT32,
                                        "float32");
}

const boost::optional<std::vector<boost::optional<double>>>&
generic_record::get_array_of_nullable_float64(
  const std::string& field_name) const
{
    return get_array_of_nullable<double>(field_name,
                                         field_kind::ARRAY_OF_FLOAT64,
                                         field_kind::ARRAY_OF_NULLABLE_FLOAT64,
                                         "float64");
}

boost::optional<std::vector<boost::optional<double>>>&
generic_record::get_array_of_nullable_float64(const std::string& field_name)
{
    return get_array_of_nullable<double>(field_name,
                                         field_kind::ARRAY_OF_FLOAT64,
                                         field_kind::ARRAY_OF_NULLABLE_FLOAT64,
                                         "float64");
}

const boost::optional<std::vector<boost::optional<std::string>>>&
generic_record::get_array_of_string(const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<std::string>>>>(
      field_name, field_kind::ARRAY_OF_STRING);
}

boost::optional<std::vector<boost::optional<std::string>>>&
generic_record::get_array_of_string(const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<std::string>>>>(
      field_name, field_kind::ARRAY_OF_STRING);
}

const boost::optional<std::vector<boost::optional<big_decimal>>>&
generic_record::get_array_of_decimal(const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<big_decimal>>>>(
      field_name, field_kind::ARRAY_OF_DECIMAL);
}

boost::optional<std::vector<boost::optional<big_decimal>>>&
generic_record::get_array_of_decimal(const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<big_decimal>>>>(
      field_name, field_kind::ARRAY_OF_DECIMAL);
}

const boost::optional<std::vector<boost::optional<local_time>>>&
generic_record::get_array_of_time(const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<local_time>>>>(
      field_name, field_kind::ARRAY_OF_TIME);
}

boost::optional<std::vector<boost::optional<local_time>>>&
generic_record::get_array_of_time(const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<local_time>>>>(
      field_name, field_kind::ARRAY_OF_TIME);
}

const boost::optional<std::vector<boost::optional<local_date>>>&
generic_record::get_array_of_date(const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<local_date>>>>(
      field_name, field_kind::ARRAY_OF_DATE);
}

boost::optional<std::vector<boost::optional<local_date>>>&
generic_record::get_array_of_date(const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<local_date>>>>(
      field_name, field_kind::ARRAY_OF_DATE);
}

const boost::optional<std::vector<boost::optional<local_date_time>>>&
generic_record::get_array_of_timestamp(const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<local_date_time>>>>(
      field_name, field_kind::ARRAY_OF_TIMESTAMP);
}

boost::optional<std::vector<boost::optional<local_date_time>>>&
generic_record::get_array_of_timestamp(const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<local_date_time>>>>(
      field_name, field_kind::ARRAY_OF_TIMESTAMP);
}

const boost::optional<std::vector<boost::optional<offset_date_time>>>&
generic_record::get_array_of_timestamp_with_timezone(
  const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<offset_date_time>>>>(
      field_name, field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE);
}

boost::optional<std::vector<boost::optional<offset_date_time>>>&
generic_record::get_array_of_timestamp_with_timezone(
  const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<offset_date_time>>>>(
      field_name, field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE);
}

const boost::optional<std::vector<boost::optional<generic_record>>>&
generic_record::get_array_of_generic_record(const std::string& field_name) const
{
    return get<boost::optional<std::vector<boost::optional<generic_record>>>>(
      field_name, field_kind::ARRAY_OF_COMPACT);
}

boost::optional<std::vector<boost::optional<generic_record>>>&
generic_record::get_array_of_generic_record(const std::string& field_name)
{
    return get<boost::optional<std::vector<boost::optional<generic_record>>>>(
      field_name, field_kind::ARRAY_OF_COMPACT);
}

boost::property_tree::ptree
write_generic_record(const generic_record& record)
{
    boost::property_tree::ptree node;

    for (const std::pair<const std::string, boost::any>& p : record.objects_) {
        const std::string& field_name = p.first;

        field_kind kind = record.get_schema().get_field(field_name)->kind;

        pimpl::field_operations::get(kind).write_json_formatted_field(
          node, record, field_name);
    }

    return node;
}

std::ostream&
operator<<(std::ostream& os, const generic_record& record)
{
    boost::property_tree::ptree pt;

    pt.put_child(record.get_schema().type_name(), write_generic_record(record));

    boost::property_tree::write_json(os, pt);

    return os;
}

bool
operator==(const generic_record& x, const generic_record& y)
{
    static const std::function<bool(const boost::any&, const boost::any&)>
      COMPARATORS[] = {
          [](const boost::any&, const boost::any&) {
              return false;
          },                                             //[0] NOT_AVAILABLE
          [](const boost::any& x, const boost::any& y) { //[1] BOOLEAN
              return boost::any_cast<bool>(x) == boost::any_cast<bool>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[2] ARRAY_OF_BOOLEAN
              return boost::any_cast<boost::optional<std::vector<bool>>>(x) ==
                     boost::any_cast<boost::optional<std::vector<bool>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[3] INT8
              return boost::any_cast<int8_t>(x) == boost::any_cast<int8_t>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[4] ARRAY_OF_INT8
              return boost::any_cast<boost::optional<std::vector<int8_t>>>(x) ==
                     boost::any_cast<boost::optional<std::vector<int8_t>>>(y);
          },
          [](const boost::any&, const boost::any&) { return false; }, // [5]
          [](const boost::any&, const boost::any&) { return false; }, // [6]
          [](const boost::any& x, const boost::any& y) { //[7] INT16
              return boost::any_cast<int16_t>(x) == boost::any_cast<int16_t>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[8] ARRAY_OF_INT16
              return boost::any_cast<boost::optional<std::vector<int16_t>>>(
                       x) ==
                     boost::any_cast<boost::optional<std::vector<int16_t>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[9] INT32
              return boost::any_cast<int32_t>(x) == boost::any_cast<int32_t>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[10] ARRAY_OF_INT32
              return boost::any_cast<boost::optional<std::vector<int32_t>>>(
                       x) ==
                     boost::any_cast<boost::optional<std::vector<int32_t>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[11] INT64
              return boost::any_cast<int64_t>(x) == boost::any_cast<int64_t>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[12] ARRAY_OF_INT64
              return boost::any_cast<boost::optional<std::vector<int64_t>>>(
                       x) ==
                     boost::any_cast<boost::optional<std::vector<int64_t>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[13] FLOAT32
              return boost::any_cast<float>(x) == boost::any_cast<float>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[14] ARRAY_OF_FLOAT32
              return boost::any_cast<boost::optional<std::vector<float>>>(x) ==
                     boost::any_cast<boost::optional<std::vector<float>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[15] FLOAT64
              return boost::any_cast<double>(x) == boost::any_cast<double>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[16] ARRAY_OF_FLOAT64
              return boost::any_cast<boost::optional<std::vector<double>>>(x) ==
                     boost::any_cast<boost::optional<std::vector<double>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[17] STRING
              return boost::any_cast<boost::optional<std::string>>(x) ==
                     boost::any_cast<boost::optional<std::string>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[18] ARRAY_OF_STRING
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<std::string>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<std::string>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[19] DECIMAL
              return boost::any_cast<boost::optional<big_decimal>>(x) ==
                     boost::any_cast<boost::optional<big_decimal>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[20] ARRAY_OF_DECIMAL
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<big_decimal>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<big_decimal>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[21] TIME
              return boost::any_cast<boost::optional<local_time>>(x) ==
                     boost::any_cast<boost::optional<local_time>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[22] ARRAY_OF_TIME
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<local_time>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<local_time>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[23] DATE
              return boost::any_cast<boost::optional<local_date>>(x) ==
                     boost::any_cast<boost::optional<local_date>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[24] ARRAY_OF_DATE
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<local_date>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<local_date>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[25] TIMESTAMP
              return boost::any_cast<boost::optional<local_date_time>>(x) ==
                     boost::any_cast<boost::optional<local_date_time>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[26] ARRAY_OF_TIMESTAMP
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<local_date_time>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<local_date_time>>>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[27] TIMESTAMP_WITH_TIMEZONE
              return boost::any_cast<boost::optional<offset_date_time>>(x) ==
                     boost::any_cast<boost::optional<offset_date_time>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[28] ARRAY_OF_TIMESTAMP_WITH_TIMEZON
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<offset_date_time>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<offset_date_time>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[29] COMPACT
              return boost::any_cast<boost::optional<generic_record>>(x) ==
                     boost::any_cast<boost::optional<generic_record>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[30] ARRAY_OF_COMPACT
              return boost::any_cast<boost::optional<
                       std::vector<boost::optional<generic_record>>>>(x) ==
                     boost::any_cast<boost::optional<
                       std::vector<boost::optional<generic_record>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { return false; }, //[31]
          [](const boost::any& x, const boost::any& y) { return false; }, //[32]
          [](const boost::any& x, const boost::any& y) { //[33] NULLABLE_BOOLEAN
              return boost::any_cast<boost::optional<bool>>(x) ==
                     boost::any_cast<boost::optional<bool>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[34] ARRAY_OF_NULLABLE_BOOLEAN
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<bool>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<bool>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[35] NULLABLE_INT8
              return boost::any_cast<boost::optional<int8_t>>(x) ==
                     boost::any_cast<boost::optional<int8_t>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[36] ARRAY_OF_NULLABLE_INT8
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<int8_t>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<int8_t>>>>(
                       y);
          },
          [](const boost::any& x, const boost::any& y) { //[37] NULLABLE_INT16
              return boost::any_cast<boost::optional<int16_t>>(x) ==
                     boost::any_cast<boost::optional<int16_t>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[38] ARRAY_OF_NULLABLE_INT16
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<int16_t>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<int16_t>>>>(
                       y);
          },
          [](const boost::any& x, const boost::any& y) { //[39] NULLABLE_INT32
              return boost::any_cast<boost::optional<int32_t>>(x) ==
                     boost::any_cast<boost::optional<int32_t>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[40] ARRAY_OF_NULLABLE_INT32
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<int32_t>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<int32_t>>>>(
                       y);
          },
          [](const boost::any& x, const boost::any& y) { //[41] NULLABLE_INT64
              return boost::any_cast<boost::optional<int64_t>>(x) ==
                     boost::any_cast<boost::optional<int64_t>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[42] ARRAY_OF_NULLABLE_INT64
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<int64_t>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<int64_t>>>>(
                       y);
          },
          [](const boost::any& x, const boost::any& y) { //[43] NULLABLE_FLOAT32
              return boost::any_cast<boost::optional<float>>(x) ==
                     boost::any_cast<boost::optional<float>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[44] ARRAY_OF_NULLABLE_FLOAT32
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<float>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<float>>>>(y);
          },
          [](const boost::any& x, const boost::any& y) { //[45] NULLABLE_FLOAT64
              return boost::any_cast<boost::optional<double>>(x) ==
                     boost::any_cast<boost::optional<double>>(y);
          },
          [](const boost::any& x,
             const boost::any& y) { //[46] ARRAY_OF_NULLABLE_FLOAT64
              return boost::any_cast<
                       boost::optional<std::vector<boost::optional<double>>>>(
                       x) ==
                     boost::any_cast<
                       boost::optional<std::vector<boost::optional<double>>>>(
                       y);
          }
      };

    const pimpl::schema& xs = x.get_schema();
    const pimpl::schema& ys = y.get_schema();

    if (xs != ys)
        return false;

    for (const std::pair<const std::string, boost::any>& xp : x.objects_) {
        const std::string& field_name = xp.first;
        const boost::any& value_of_x = xp.second;
        const boost::any& value_of_y = y.objects_.at(field_name);

        boost::optional<pimpl::field_descriptor> kind_opt =
          xs.get_field(field_name);

        if (!COMPARATORS[std::size_t(kind_opt->kind)](value_of_x, value_of_y))
            return false;
    }

    return true;
}

bool
operator!=(const generic_record& x, const generic_record& y)
{
    return !(x == y);
}
} // namespace generic_record

std::ostream&
operator<<(std::ostream& os, field_kind kind)
{
    switch (kind) {
        case field_kind::NOT_AVAILABLE:
            os << "NOT_AVAILABLE";
            break;
        case field_kind::BOOLEAN:
            os << "BOOLEAN";
            break;
        case field_kind::ARRAY_OF_BOOLEAN:
            os << "ARRAY_OF_BOOLEAN";
            break;
        case field_kind::INT8:
            os << "INT8";
            break;
        case field_kind::ARRAY_OF_INT8:
            os << "ARRAY_OF_INT8";
            break;
        case field_kind::INT16:
            os << "INT16";
            break;
        case field_kind::ARRAY_OF_INT16:
            os << "ARRAY_OF_INT16";
            break;
        case field_kind::INT32:
            os << "INT32";
            break;
        case field_kind::ARRAY_OF_INT32:
            os << "ARRAY_OF_INT32";
            break;
        case field_kind::INT64:
            os << "INT64";
            break;
        case field_kind::ARRAY_OF_INT64:
            os << "ARRAY_OF_INT64";
            break;
        case field_kind::FLOAT32:
            os << "FLOAT32";
            break;
        case field_kind::ARRAY_OF_FLOAT32:
            os << "ARRAY_OF_FLOAT32";
            break;
        case field_kind::FLOAT64:
            os << "FLOAT64";
            break;
        case field_kind::ARRAY_OF_FLOAT64:
            os << "ARRAY_OF_FLOAT64";
            break;
        case field_kind::STRING:
            os << "STRING";
            break;
        case field_kind::ARRAY_OF_STRING:
            os << "ARRAY_OF_STRING";
            break;
        case field_kind::DECIMAL:
            os << "DECIMAL";
            break;
        case field_kind::ARRAY_OF_DECIMAL:
            os << "ARRAY_OF_DECIMAL";
            break;
        case field_kind::TIME:
            os << "TIME";
            break;
        case field_kind::ARRAY_OF_TIME:
            os << "ARRAY_OF_TIME";
            break;
        case field_kind::DATE:
            os << "DATE";
            break;
        case field_kind::ARRAY_OF_DATE:
            os << "ARRAY_OF_DATE";
            break;
        case field_kind::TIMESTAMP:
            os << "TIMESTAMP";
            break;
        case field_kind::ARRAY_OF_TIMESTAMP:
            os << "ARRAY_OF_TIMESTAMP";
            break;
        case field_kind::TIMESTAMP_WITH_TIMEZONE:
            os << "TIMESTAMP_WITH_TIMEZONE";
            break;
        case field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE:
            os << "ARRAY_OF_TIMESTAMP_WITH_TIMEZONE";
            break;
        case field_kind::COMPACT:
            os << "COMPACT";
            break;
        case field_kind::ARRAY_OF_COMPACT:
            os << "ARRAY_OF_COMPACT";
            break;
        case field_kind::NULLABLE_BOOLEAN:
            os << "NULLABLE_BOOLEAN";
            break;
        case field_kind::ARRAY_OF_NULLABLE_BOOLEAN:
            os << "ARRAY_OF_NULLABLE_BOOLEAN";
            break;
        case field_kind::NULLABLE_INT8:
            os << "NULLABLE_INT8";
            break;
        case field_kind::ARRAY_OF_NULLABLE_INT8:
            os << "ARRAY_OF_NULLABLE_INT8";
            break;
        case field_kind::NULLABLE_INT16:
            os << "NULLABLE_INT16";
            break;
        case field_kind::ARRAY_OF_NULLABLE_INT16:
            os << "ARRAY_OF_NULLABLE_INT16";
            break;
        case field_kind::NULLABLE_INT32:
            os << "NULLABLE_INT32";
            break;
        case field_kind::ARRAY_OF_NULLABLE_INT32:
            os << "ARRAY_OF_NULLABLE_INT32";
            break;
        case field_kind::NULLABLE_INT64:
            os << "NULLABLE_INT64";
            break;
        case field_kind::ARRAY_OF_NULLABLE_INT64:
            os << "ARRAY_OF_NULLABLE_INT64";
            break;
        case field_kind::NULLABLE_FLOAT32:
            os << "NULLABLE_FLOAT32";
            break;
        case field_kind::ARRAY_OF_NULLABLE_FLOAT32:
            os << "ARRAY_OF_NULLABLE_FLOAT32";
            break;
        case field_kind::NULLABLE_FLOAT64:
            os << "NULLABLE_FLOAT64";
            break;
        case field_kind::ARRAY_OF_NULLABLE_FLOAT64:
            os << "ARRAY_OF_NULLABLE_FLOAT64";
            break;
    }

    return os;
}

} // namespace serialization
} // namespace client
} // namespace hazelcast

namespace hazelcast {
namespace client {
namespace serialization {
namespace compact {

compact_writer::compact_writer(
  pimpl::default_compact_writer* default_compact_writer)
  : default_compact_writer(default_compact_writer)
  , schema_writer(nullptr)
{}

compact_writer::compact_writer(pimpl::schema_writer* schema_writer)
  : default_compact_writer(nullptr)
  , schema_writer(schema_writer)
{}

void
compact_writer::write_boolean(const std::string& field_name, bool value)
{
    if (default_compact_writer) {
        default_compact_writer->write_boolean(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::BOOLEAN);
    }
}
void
compact_writer::write_int8(const std::string& field_name, int8_t value)
{
    if (default_compact_writer) {
        default_compact_writer->write_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::INT8);
    }
}

void
compact_writer::write_int16(const std::string& field_name, int16_t value)
{
    if (default_compact_writer) {
        default_compact_writer->write_int16(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::INT16);
    }
}

void
compact_writer::write_int32(const std::string& field_name, int32_t value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::INT32);
    }
}

void
compact_writer::write_int64(const std::string& field_name, int64_t value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_int64(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::INT64);
    }
}

void
compact_writer::write_float32(const std::string& field_name, float value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_float32(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::FLOAT32);
    }
}

void
compact_writer::write_float64(const std::string& field_name, double value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_float64(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::FLOAT64);
    }
}

void
compact_writer::write_string(const std::string& field_name,
                             const boost::optional<std::string>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_string(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::STRING);
    }
}

void
compact_writer::write_decimal(const std::string& field_name,
                              const boost::optional<big_decimal>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_decimal(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::DECIMAL);
    }
}

void
compact_writer::write_time(
  const std::string& field_name,
  const boost::optional<hazelcast::client::local_time>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_time(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::TIME);
    }
}

void
compact_writer::write_date(
  const std::string& field_name,
  const boost::optional<hazelcast::client::local_date>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_date(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::DATE);
    }
}

void
compact_writer::write_timestamp(
  const std::string& field_name,
  const boost::optional<hazelcast::client::local_date_time>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_timestamp(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::TIMESTAMP);
    }
}

void
compact_writer::write_timestamp_with_timezone(
  const std::string& field_name,
  const boost::optional<hazelcast::client::offset_date_time>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_timestamp_with_timezone(field_name,
                                                              value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::TIMESTAMP_WITH_TIMEZONE);
    }
}

void
compact_writer::write_array_of_boolean(
  const std::string& field_name,
  const boost::optional<std::vector<bool>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_boolean(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_BOOLEAN);
    }
}

void
compact_writer::write_array_of_int8(
  const std::string& field_name,
  const boost::optional<std::vector<int8_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_INT8);
    }
}

void
compact_writer::write_array_of_int16(
  const std::string& field_name,
  const boost::optional<std::vector<int16_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int16(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_INT16);
    }
}

void
compact_writer::write_array_of_int32(
  const std::string& field_name,
  const boost::optional<std::vector<int32_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_INT32);
    }
}

void
compact_writer::write_array_of_int64(
  const std::string& field_name,
  const boost::optional<std::vector<int64_t>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_int64(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_INT64);
    }
}

void
compact_writer::write_array_of_float32(
  const std::string& field_name,
  const boost::optional<std::vector<float>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_float32(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_FLOAT32);
    }
}

void
compact_writer::write_array_of_float64(
  const std::string& field_name,
  const boost::optional<std::vector<double>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_float64(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_FLOAT64);
    }
}

void
compact_writer::write_array_of_string(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<std::string>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_string(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_STRING);
    }
}

void
compact_writer::write_array_of_decimal(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<big_decimal>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_decimal(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_DECIMAL);
    }
}

void
compact_writer::write_array_of_time(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<local_time>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_time(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_TIME);
    }
}

void
compact_writer::write_array_of_date(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<local_date>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_date(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_DATE);
    }
}

void
compact_writer::write_array_of_timestamp(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<local_date_time>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_timestamp(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::ARRAY_OF_TIMESTAMP);
    }
}

void
compact_writer::write_array_of_timestamp_with_timezone(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<offset_date_time>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_timestamp_with_timezone(
          field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE);
    }
}

void
compact_writer::write_nullable_boolean(const std::string& field_name,
                                       const boost::optional<bool>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_boolean(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_BOOLEAN);
    }
}

void
compact_writer::write_nullable_int8(const std::string& field_name,
                                    const boost::optional<int8_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_INT8);
    }
}

void
compact_writer::write_nullable_int16(const std::string& field_name,
                                     const boost::optional<int16_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int16(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_INT16);
    }
}

void
compact_writer::write_nullable_int32(const std::string& field_name,
                                     const boost::optional<int32_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int32(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_INT32);
    }
}

void
compact_writer::write_nullable_int64(const std::string& field_name,
                                     const boost::optional<int64_t>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_int64(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_INT64);
    }
}

void
compact_writer::write_nullable_float32(const std::string& field_name,
                                       const boost::optional<float>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_float32(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_FLOAT32);
    }
}

void
compact_writer::write_nullable_float64(const std::string& field_name,
                                       const boost::optional<double>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_nullable_float64(field_name, value);
    } else {
        schema_writer->add_field(field_name, field_kind::NULLABLE_FLOAT64);
    }
}

void
compact_writer::write_array_of_nullable_boolean(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<bool>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_boolean(field_name,
                                                                value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_BOOLEAN);
    }
}

void
compact_writer::write_array_of_nullable_int8(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int8_t>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_int8(field_name, value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_INT8);
    }
}

void
compact_writer::write_array_of_nullable_int16(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int16_t>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_int16(field_name,
                                                              value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_INT16);
    }
}

void
compact_writer::write_array_of_nullable_int32(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int32_t>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_int32(field_name,
                                                              value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_INT32);
    }
}

void
compact_writer::write_array_of_nullable_int64(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int64_t>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_int64(field_name,
                                                              value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_INT64);
    }
}

void
compact_writer::write_array_of_nullable_float32(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<float>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_float32(field_name,
                                                                value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_FLOAT32);
    }
}

void
compact_writer::write_array_of_nullable_float64(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<double>>>& value)
{
    if (default_compact_writer != nullptr) {
        default_compact_writer->write_array_of_nullable_float64(field_name,
                                                                value);
    } else {
        schema_writer->add_field(field_name,
                                 field_kind::ARRAY_OF_NULLABLE_FLOAT64);
    }
}

} // namespace compact

namespace pimpl {

compact::compact_reader
create_compact_reader(
  pimpl::compact_stream_serializer& compact_stream_serializer,
  object_data_input& object_data_input,
  const pimpl::schema& schema)
{
    return compact::compact_reader{ compact_stream_serializer,
                                    object_data_input,
                                    schema };
}

} // namespace pimpl

namespace compact {

const compact_reader::offset_func compact_reader::BYTE_OFFSET_READER =
  pimpl::offset_reader::get_offset<int8_t>;
const compact_reader::offset_func compact_reader::SHORT_OFFSET_READER =
  pimpl::offset_reader::get_offset<int16_t>;
const compact_reader::offset_func compact_reader::INT_OFFSET_READER =
  pimpl::offset_reader::get_offset<int32_t>;

compact_reader::compact_reader(
  pimpl::compact_stream_serializer& compact_stream_serializer,
  serialization::object_data_input& object_data_input,
  const pimpl::schema& schema)
  : compact_stream_serializer(compact_stream_serializer)
  , object_data_input(object_data_input)
  , schema(schema)
{
    size_t final_position;
    size_t number_of_var_size_fields = schema.number_of_var_size_fields();
    if (number_of_var_size_fields != 0) {
        uint32_t data_length = object_data_input.read<int32_t>();
        data_start_position = object_data_input.position();
        variable_offsets_position = data_start_position + data_length;
        if (data_length < pimpl::offset_reader::BYTE_OFFSET_READER_RANGE) {
            get_offset = BYTE_OFFSET_READER;
            final_position =
              variable_offsets_position + number_of_var_size_fields;
        } else if (data_length <
                   pimpl::offset_reader::SHORT_OFFSET_READER_RANGE) {
            get_offset = SHORT_OFFSET_READER;
            final_position =
              variable_offsets_position +
              (number_of_var_size_fields * util::Bits::SHORT_SIZE_IN_BYTES);
        } else {
            get_offset = INT_OFFSET_READER;
            final_position =
              variable_offsets_position +
              (number_of_var_size_fields * util::Bits::INT_SIZE_IN_BYTES);
        }
    } else {
        get_offset = INT_OFFSET_READER;
        variable_offsets_position = 0;
        data_start_position = object_data_input.position();
        final_position =
          data_start_position + schema.fixed_size_fields_length();
    }
    // set the position to final so that the next one to read something from
    // `in` can start from correct position
    object_data_input.position(static_cast<int>(final_position));
}

bool
compact_reader::is_field_exists(const std::string& field_name,
                                field_kind kind) const
{
    const auto& fields = schema.fields();
    const auto& field_descriptor = fields.find(field_name);
    if (field_descriptor == fields.end()) {
        return false;
    }
    return field_descriptor->second.kind == kind;
}

const pimpl::field_descriptor&
compact_reader::get_field_descriptor(const std::string& field_name) const
{
    const auto& fields = schema.fields();
    const auto& field_descriptor = fields.find(field_name);
    if (field_descriptor == fields.end()) {
        BOOST_THROW_EXCEPTION(unknown_field(field_name));
    }
    return field_descriptor->second;
}

const pimpl::field_descriptor&
compact_reader::get_field_descriptor(const std::string& field_name,
                                     field_kind kind) const
{
    const auto& field_descriptor = get_field_descriptor(field_name);
    if (field_descriptor.kind != kind) {
        BOOST_THROW_EXCEPTION(
          unexpected_field_kind(field_descriptor.kind, field_name));
    }
    return field_descriptor;
}

std::function<int32_t(serialization::object_data_input&, uint32_t, uint32_t)>
compact_reader::get_offset_reader(int32_t data_length)
{
    if (data_length < pimpl::offset_reader::BYTE_OFFSET_READER_RANGE) {
        return BYTE_OFFSET_READER;
    } else if (data_length < pimpl::offset_reader::SHORT_OFFSET_READER_RANGE) {
        return SHORT_OFFSET_READER;
    } else {
        return INT_OFFSET_READER;
    }
}

exception::hazelcast_serialization
compact_reader::unexpected_null_value_in_array(const std::string& field_name,
                                               const std::string& method_suffix)
{
    return {
        "compact_reader",
        (boost::format(
           "Error while reading %1%. Array with null items can not be read via "
           "read_array_of_%2% methods. Use read_array_of_nullable_%2% "
           "instead") %
         field_name % method_suffix)
          .str()
    };
}
exception::hazelcast_serialization
compact_reader::unknown_field(const std::string& field_name) const
{
    return { "compact_reader",
             (boost::format("Unknown field name %1% on %2% ") % field_name %
              schema)
               .str() };
}

exception::hazelcast_serialization
compact_reader::unexpected_field_kind(field_kind kind,
                                      const std::string& field_name) const
{
    return { "compact_reader",
             (boost::format("Unexpected fieldKind %1% for %2% on %3%") % kind %
              field_name % schema)
               .str() };
}

exception::hazelcast_serialization
compact_reader::unexpected_null_value(const std::string& field_name,
                                      const std::string& method_suffix)
{
    return { "compact_reader",
             (boost::format(
                "Error while reading %1%. null value can not be read via "
                "read_%2% methods. Use read_nullable_%2%  instead.") %
              field_name % method_suffix)
               .str() };
}

size_t
compact_reader::read_fixed_size_position(
  const pimpl::field_descriptor& field_descriptor) const
{
    int32_t primitive_offset = field_descriptor.offset;
    return primitive_offset + data_start_position;
}

int32_t
compact_reader::read_var_size_position(
  const pimpl::field_descriptor& field_descriptor) const
{
    int32_t index = field_descriptor.index;
    int32_t offset =
      get_offset(object_data_input, variable_offsets_position, index);
    return offset == pimpl::offset_reader::NULL_OFFSET
             ? pimpl::offset_reader::NULL_OFFSET
             : offset + data_start_position;
}

bool
compact_reader::read_boolean(const std::string& fieldName)
{
    return read_primitive<bool>(
      fieldName, field_kind::BOOLEAN, field_kind::NULLABLE_BOOLEAN, "boolean");
}

int8_t
compact_reader::read_int8(const std::string& fieldName)
{
    return read_primitive<int8_t>(
      fieldName, field_kind::INT8, field_kind::NULLABLE_INT8, "int8");
}

int16_t
compact_reader::read_int16(const std::string& field_name)
{
    return read_primitive<int16_t>(
      field_name, field_kind::INT16, field_kind::NULLABLE_INT16, "int16");
}

int32_t
compact_reader::read_int32(const std::string& field_name)
{
    return read_primitive<int32_t>(
      field_name, field_kind::INT32, field_kind::NULLABLE_INT32, "int32");
}

int64_t
compact_reader::read_int64(const std::string& field_name)
{
    return read_primitive<int64_t>(
      field_name, field_kind::INT64, field_kind::NULLABLE_INT64, "int64");
}

float
compact_reader::read_float32(const std::string& field_name)
{
    return read_primitive<float>(
      field_name, field_kind::FLOAT32, field_kind::NULLABLE_FLOAT32, "float32");
}

double
compact_reader::read_float64(const std::string& field_name)
{
    return read_primitive<double>(
      field_name, field_kind::FLOAT64, field_kind::NULLABLE_FLOAT64, "float64");
}

boost::optional<std::string>
compact_reader::read_string(const std::string& field_name)
{
    return read_variable_size<std::string>(field_name, field_kind::STRING);
}

boost::optional<big_decimal>
compact_reader::read_decimal(const std::string& field_name)
{
    return read_variable_size<big_decimal>(field_name, field_kind::DECIMAL);
}

boost::optional<hazelcast::client::local_time>
compact_reader::read_time(const std::string& field_name)
{
    return read_variable_size<hazelcast::client::local_time>(field_name,
                                                             field_kind::TIME);
}

boost::optional<hazelcast::client::local_date>
compact_reader::read_date(const std::string& field_name)
{
    return read_variable_size<hazelcast::client::local_date>(field_name,
                                                             field_kind::DATE);
}

boost::optional<hazelcast::client::local_date_time>
compact_reader::read_timestamp(const std::string& field_name)
{
    return read_variable_size<hazelcast::client::local_date_time>(
      field_name, field_kind::TIMESTAMP);
}

boost::optional<hazelcast::client::offset_date_time>
compact_reader::read_timestamp_with_timezone(const std::string& field_name)
{
    return read_variable_size<hazelcast::client::offset_date_time>(
      field_name, field_kind::TIMESTAMP_WITH_TIMEZONE);
}

boost::optional<std::vector<bool>>
compact_reader::read_array_of_boolean(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<bool>>(
      field_name,
      field_kind::ARRAY_OF_BOOLEAN,
      field_kind::ARRAY_OF_NULLABLE_BOOLEAN,
      "boolean");
}

boost::optional<std::vector<int8_t>>
compact_reader::read_array_of_int8(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int8_t>>(
      field_name,
      field_kind::ARRAY_OF_INT8,
      field_kind::ARRAY_OF_NULLABLE_INT8,
      "int8");
}

boost::optional<std::vector<int16_t>>
compact_reader::read_array_of_int16(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int16_t>>(
      field_name,
      field_kind::ARRAY_OF_INT16,
      field_kind::ARRAY_OF_NULLABLE_INT16,
      "int16");
}

boost::optional<std::vector<int32_t>>
compact_reader::read_array_of_int32(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int32_t>>(
      field_name,
      field_kind::ARRAY_OF_INT32,
      field_kind::ARRAY_OF_NULLABLE_INT32,
      "int32");
}
boost::optional<std::vector<int64_t>>
compact_reader::read_array_of_int64(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<int64_t>>(
      field_name,
      field_kind::ARRAY_OF_INT64,
      field_kind::ARRAY_OF_NULLABLE_INT64,
      "int64");
}

boost::optional<std::vector<float>>
compact_reader::read_array_of_float32(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<float>>(
      field_name,
      field_kind::ARRAY_OF_FLOAT32,
      field_kind::ARRAY_OF_NULLABLE_FLOAT32,
      "float32");
}

boost::optional<std::vector<double>>
compact_reader::read_array_of_float64(const std::string& field_name)
{
    return read_array_of_primitive<std::vector<double>>(
      field_name,
      field_kind::ARRAY_OF_FLOAT64,
      field_kind::ARRAY_OF_NULLABLE_FLOAT64,
      "float64");
}

boost::optional<std::vector<boost::optional<std::string>>>
compact_reader::read_array_of_string(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, field_kind::ARRAY_OF_STRING);
    return read_array_of_variable_size<std::string>(descriptor);
}

boost::optional<std::vector<boost::optional<big_decimal>>>
compact_reader::read_array_of_decimal(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, field_kind::ARRAY_OF_DECIMAL);
    return read_array_of_variable_size<big_decimal>(descriptor);
}

boost::optional<std::vector<boost::optional<local_time>>>
compact_reader::read_array_of_time(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, field_kind::ARRAY_OF_TIME);
    return read_array_of_variable_size<local_time>(descriptor);
}

boost::optional<std::vector<boost::optional<local_date>>>
compact_reader::read_array_of_date(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, field_kind::ARRAY_OF_DATE);
    return read_array_of_variable_size<local_date>(descriptor);
}

boost::optional<std::vector<boost::optional<local_date_time>>>
compact_reader::read_array_of_timestamp(const std::string& field_name)
{
    const auto& descriptor =
      get_field_descriptor(field_name, field_kind::ARRAY_OF_TIMESTAMP);
    return read_array_of_variable_size<local_date_time>(descriptor);
}

boost::optional<std::vector<boost::optional<offset_date_time>>>
compact_reader::read_array_of_timestamp_with_timezone(
  const std::string& field_name)
{
    const auto& descriptor = get_field_descriptor(
      field_name, field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE);
    return read_array_of_variable_size<offset_date_time>(descriptor);
}

boost::optional<bool>
compact_reader::read_nullable_boolean(const std::string& field_name)
{
    return read_nullable_primitive<bool>(
      field_name, field_kind::BOOLEAN, field_kind::NULLABLE_BOOLEAN);
}

boost::optional<int8_t>
compact_reader::read_nullable_int8(const std::string& field_name)
{
    return read_nullable_primitive<int8_t>(
      field_name, field_kind::INT8, field_kind::NULLABLE_INT8);
}

boost::optional<int16_t>
compact_reader::read_nullable_int16(const std::string& field_name)
{
    return read_nullable_primitive<int16_t>(
      field_name, field_kind::INT16, field_kind::NULLABLE_INT16);
}

boost::optional<int32_t>
compact_reader::read_nullable_int32(const std::string& field_name)
{
    return read_nullable_primitive<int32_t>(
      field_name, field_kind::INT32, field_kind::NULLABLE_INT32);
}

boost::optional<int64_t>
compact_reader::read_nullable_int64(const std::string& field_name)
{
    return read_nullable_primitive<int64_t>(
      field_name, field_kind::INT64, field_kind::NULLABLE_INT64);
}

boost::optional<float>
compact_reader::read_nullable_float32(const std::string& field_name)
{
    return read_nullable_primitive<float>(
      field_name, field_kind::FLOAT32, field_kind::NULLABLE_FLOAT32);
}

boost::optional<double>
compact_reader::read_nullable_float64(const std::string& field_name)
{
    return read_nullable_primitive<double>(
      field_name, field_kind::FLOAT64, field_kind::NULLABLE_FLOAT64);
}

boost::optional<std::vector<boost::optional<bool>>>
compact_reader::read_array_of_nullable_boolean(const std::string& field_name)
{
    return read_array_of_nullable<bool>(field_name,
                                        field_kind::ARRAY_OF_BOOLEAN,
                                        field_kind::ARRAY_OF_NULLABLE_BOOLEAN);
}

boost::optional<std::vector<boost::optional<int8_t>>>
compact_reader::read_array_of_nullable_int8(const std::string& field_name)
{
    return read_array_of_nullable<int8_t>(field_name,
                                          field_kind::ARRAY_OF_INT8,
                                          field_kind::ARRAY_OF_NULLABLE_INT8);
}

boost::optional<std::vector<boost::optional<int16_t>>>
compact_reader::read_array_of_nullable_int16(const std::string& field_name)
{
    return read_array_of_nullable<int16_t>(field_name,
                                           field_kind::ARRAY_OF_INT16,
                                           field_kind::ARRAY_OF_NULLABLE_INT16);
}

boost::optional<std::vector<boost::optional<int32_t>>>
compact_reader::read_array_of_nullable_int32(const std::string& field_name)
{
    return read_array_of_nullable<int32_t>(field_name,
                                           field_kind::ARRAY_OF_INT32,
                                           field_kind::ARRAY_OF_NULLABLE_INT32);
}

boost::optional<std::vector<boost::optional<int64_t>>>
compact_reader::read_array_of_nullable_int64(const std::string& field_name)
{
    return read_array_of_nullable<int64_t>(field_name,
                                           field_kind::ARRAY_OF_INT64,
                                           field_kind::ARRAY_OF_NULLABLE_INT64);
}

boost::optional<std::vector<boost::optional<float>>>
compact_reader::read_array_of_nullable_float32(const std::string& field_name)
{
    return read_array_of_nullable<float>(field_name,
                                         field_kind::ARRAY_OF_FLOAT32,
                                         field_kind::ARRAY_OF_NULLABLE_FLOAT32);
}

boost::optional<std::vector<boost::optional<double>>>
compact_reader::read_array_of_nullable_float64(const std::string& field_name)
{
    return read_array_of_nullable<double>(
      field_name,
      field_kind::ARRAY_OF_FLOAT64,
      field_kind::ARRAY_OF_NULLABLE_FLOAT64);
}

} // namespace compact

namespace pimpl {

compact::compact_writer
create_compact_writer(pimpl::default_compact_writer* default_compact_writer)
{
    return compact::compact_writer{ default_compact_writer };
}

compact::compact_writer
create_compact_writer(pimpl::schema_writer* schema_writer)
{
    return compact::compact_writer{ schema_writer };
}

default_compact_writer::default_compact_writer(
  compact_stream_serializer& compact_stream_serializer,
  object_data_output& object_data_output,
  const schema& schema)
  : compact_stream_serializer_(compact_stream_serializer)
  , object_data_output_(object_data_output)
  , schema_(schema)
  , field_offsets(schema.number_of_var_size_fields())
{
    if (schema.number_of_var_size_fields() != 0) {
        data_start_position =
          object_data_output_.position() + util::Bits::INT_SIZE_IN_BYTES;
        // Skip for length and primitives.
        object_data_output_.write_zero_bytes(schema.fixed_size_fields_length() +
                                             util::Bits::INT_SIZE_IN_BYTES);
    } else {
        data_start_position = object_data_output_.position();
        // Skip for primitives. No need to write data length, when there is no
        // variable-size fields.
        object_data_output_.write_zero_bytes(schema.fixed_size_fields_length());
    }
}

void
default_compact_writer::write_boolean(const std::string& field_name, bool value)
{
    field_descriptor descriptor =
      check_field_definition(field_name, field_kind::BOOLEAN);
    int32_t offset_in_bytes = descriptor.offset;
    int8_t offset_in_bits = descriptor.bit_offset;
    size_t write_offset = offset_in_bytes + data_start_position;
    object_data_output_.write_boolean_bit_at(
      write_offset, offset_in_bits, value);
}

void
default_compact_writer::write_int8(const std::string& field_name, int8_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT8);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_int16(const std::string& field_name,
                                    int16_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT16);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_int32(const std::string& field_name,
                                    int32_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT32);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_int64(const std::string& field_name,
                                    int64_t value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::INT64);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_float32(const std::string& field_name,
                                      float value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::FLOAT32);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_float64(const std::string& field_name,
                                      double value)
{
    size_t position =
      get_fixed_size_field_position(field_name, field_kind::FLOAT64);
    object_data_output_.write_at(position, value);
}

void
default_compact_writer::write_string(const std::string& field_name,
                                     const boost::optional<std::string>& value)
{
    write_variable_size_field(field_name, field_kind::STRING, value);
}

void
default_compact_writer::write_decimal(const std::string& field_name,
                                      const boost::optional<big_decimal>& value)
{
    write_variable_size_field(field_name, field_kind::DECIMAL, value);
}

void
default_compact_writer::write_time(
  const std::string& field_name,
  const boost::optional<hazelcast::client::local_time>& value)
{
    write_variable_size_field(field_name, field_kind::TIME, value);
}
void
default_compact_writer::write_date(
  const std::string& field_name,
  const boost::optional<hazelcast::client::local_date>& value)
{
    write_variable_size_field(field_name, field_kind::DATE, value);
}

void
default_compact_writer::write_timestamp(
  const std::string& field_name,
  const boost::optional<hazelcast::client::local_date_time>& value)
{
    write_variable_size_field(field_name, field_kind::TIMESTAMP, value);
}

void
default_compact_writer::write_timestamp_with_timezone(
  const std::string& field_name,
  const boost::optional<hazelcast::client::offset_date_time>& value)
{
    write_variable_size_field(
      field_name, field_kind::TIMESTAMP_WITH_TIMEZONE, value);
}

void
default_compact_writer::write_array_of_boolean(
  const std::string& field_name,
  const boost::optional<std::vector<bool>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_BOOLEAN, value);
}

void
default_compact_writer::write_array_of_int8(
  const std::string& field_name,
  const boost::optional<std::vector<int8_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT8, value);
}

void
default_compact_writer::write_array_of_int16(
  const std::string& field_name,
  const boost::optional<std::vector<int16_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT16, value);
}

void
default_compact_writer::write_array_of_int32(
  const std::string& field_name,
  const boost::optional<std::vector<int32_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT32, value);
}

void
default_compact_writer::write_array_of_int64(
  const std::string& field_name,
  const boost::optional<std::vector<int64_t>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_INT64, value);
}

void
default_compact_writer::write_array_of_float32(
  const std::string& field_name,
  const boost::optional<std::vector<float>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_FLOAT32, value);
}

void
default_compact_writer::write_array_of_float64(
  const std::string& field_name,
  const boost::optional<std::vector<double>>& value)
{
    write_variable_size_field(field_name, field_kind::ARRAY_OF_FLOAT64, value);
}

void
default_compact_writer::write_array_of_string(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<std::string>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_STRING, value);
}

void
default_compact_writer::write_array_of_decimal(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<big_decimal>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_DECIMAL, value);
}

void
default_compact_writer::write_array_of_time(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<local_time>>>& value)
{
    write_array_of_variable_size(field_name, field_kind::ARRAY_OF_TIME, value);
}

void
default_compact_writer::write_array_of_date(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<local_date>>>& value)
{
    write_array_of_variable_size(field_name, field_kind::ARRAY_OF_DATE, value);
}

void
default_compact_writer::write_array_of_timestamp(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<local_date_time>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_TIMESTAMP, value);
}

void
default_compact_writer::write_array_of_timestamp_with_timezone(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<offset_date_time>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_TIMESTAMP_WITH_TIMEZONE, value);
}

void
default_compact_writer::write_nullable_boolean(
  const std::string& field_name,
  const boost::optional<bool>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_BOOLEAN, value);
}
void
default_compact_writer::write_nullable_int8(
  const std::string& field_name,
  const boost::optional<int8_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT8, value);
}
void
default_compact_writer::write_nullable_int16(
  const std::string& field_name,
  const boost::optional<int16_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT16, value);
}
void
default_compact_writer::write_nullable_int32(
  const std::string& field_name,
  const boost::optional<int32_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT32, value);
}
void
default_compact_writer::write_nullable_int64(
  const std::string& field_name,
  const boost::optional<int64_t>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_INT64, value);
}
void
default_compact_writer::write_nullable_float32(
  const std::string& field_name,
  const boost::optional<float>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_FLOAT32, value);
}
void
default_compact_writer::write_nullable_float64(
  const std::string& field_name,
  const boost::optional<double>& value)
{
    write_variable_size_field(field_name, field_kind::NULLABLE_FLOAT64, value);
}

void
default_compact_writer::write_array_of_nullable_boolean(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<bool>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_BOOLEAN, value);
}

void
default_compact_writer::write_array_of_nullable_int8(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int8_t>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_INT8, value);
}

void
default_compact_writer::write_array_of_nullable_int16(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int16_t>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_INT16, value);
}

void
default_compact_writer::write_array_of_nullable_int32(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int32_t>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_INT32, value);
}

void
default_compact_writer::write_array_of_nullable_int64(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<int64_t>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_INT64, value);
}

void
default_compact_writer::write_array_of_nullable_float32(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<float>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_FLOAT32, value);
}

void
default_compact_writer::write_array_of_nullable_float64(
  const std::string& field_name,
  const boost::optional<std::vector<boost::optional<double>>>& value)
{
    write_array_of_variable_size(
      field_name, field_kind::ARRAY_OF_NULLABLE_FLOAT64, value);
}

void
default_compact_writer::end()
{
    if (schema_.number_of_var_size_fields() == 0) {
        // There are no variable size fields
        return;
    }
    size_t position = object_data_output_.position();
    size_t data_length = position - data_start_position;
    write_offsets(data_length, field_offsets);
    // write dataLength
    object_data_output_.write_at(data_start_position -
                                   util::Bits::INT_SIZE_IN_BYTES,
                                 (int32_t)data_length);
}

size_t
default_compact_writer::get_fixed_size_field_position(
  const std::string& field_name,
  enum field_kind field_kind) const
{
    const field_descriptor& field_descriptor =
      check_field_definition(field_name, field_kind);
    return field_descriptor.offset + data_start_position;
}

const field_descriptor&
default_compact_writer::check_field_definition(const std::string& field_name,
                                               field_kind kind) const
{
    auto iterator = schema_.fields().find(field_name);
    if (iterator == schema_.fields().end()) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "default_compact_writer",
          (boost::format("Invalid field name %1% for %2%") % field_name %
           schema_)
            .str()));
    }
    if (iterator->second.kind != kind) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization(
          "default_compact_writer",
          (boost::format("Invalid field type %1% for %2%") % field_name %
           schema_)
            .str()));
    }
    return iterator->second;
}

void
default_compact_writer::write_offsets(size_t data_length,
                                      const std::vector<int32_t>& offsets)
{
    if (data_length < offset_reader::BYTE_OFFSET_READER_RANGE) {
        for (int32_t offset : offsets) {
            object_data_output_.write<int8_t>(static_cast<int8_t>(offset));
        }
    } else if (data_length < offset_reader::SHORT_OFFSET_READER_RANGE) {
        for (int32_t offset : offsets) {
            object_data_output_.write<int16_t>(static_cast<int16_t>(offset));
        }
    } else {
        for (int32_t offset : offsets) {
            object_data_output_.write<int32_t>(offset);
        }
    }
}

void
default_compact_writer::set_position(const std::string& field_name,
                                     enum field_kind field_kind)
{
    const auto& field_descriptor =
      check_field_definition(field_name, field_kind);
    size_t pos = object_data_output_.position();
    size_t fieldPosition = pos - data_start_position;
    int index = field_descriptor.index;
    field_offsets[index] = static_cast<int32_t>(fieldPosition);
}

void
default_compact_writer::set_position_as_null(const std::string& field_name,
                                             enum field_kind field_kind)
{
    const auto& field_descriptor =
      check_field_definition(field_name, field_kind);
    int index = field_descriptor.index;
    field_offsets[index] = -1;
}

std::array<uint64_t, 256>
init_fp_table()
{
    std::array<uint64_t, 256> FP_TABLE;
    for (int i = 0; i < 256; ++i) {
        uint64_t fp = i;
        for (int j = 0; j < 8; ++j) {
            fp = (fp >> 1) ^ (rabin_finger_print::INIT & -(fp & 1L));
        }
        FP_TABLE[i] = fp;
    }
    return FP_TABLE;
}

constexpr uint64_t rabin_finger_print::INIT;

uint64_t
rabin_finger_print::fingerprint64(uint64_t fp, byte b)
{
    static std::array<uint64_t, 256> FP_TABLE = init_fp_table();
    return (fp >> 8) ^ FP_TABLE[(int)(fp ^ b) & 0xff];
}

uint64_t
rabin_finger_print::fingerprint64(uint64_t fp, int32_t v)
{
    fp = fingerprint64(fp, (byte)((v)&0xFF));
    fp = fingerprint64(fp, (byte)((v >> 8) & 0xFF));
    fp = fingerprint64(fp, (byte)((v >> 16) & 0xFF));
    fp = fingerprint64(fp, (byte)((v >> 24) & 0xFF));
    return fp;
}

uint64_t
rabin_finger_print::fingerprint64(uint64_t fp, const std::string& value)
{
    fp = fingerprint64(fp, (int)value.size());
    for (const auto& item : value) {
        fp = fingerprint64(fp, (byte)item);
    }
    return fp;
}

/**
 * Calculates the fingerprint of the schema from its type name and fields.
 */
int64_t
rabin_finger_print::fingerprint64(
  const std::string& type_name,
  std::map<std::string, field_descriptor>& fields)
{
    uint64_t fingerPrint = fingerprint64(INIT, type_name);
    fingerPrint = fingerprint64(fingerPrint, (int)fields.size());
    for (const auto& entry : fields) {
        const field_descriptor& descriptor = entry.second;
        fingerPrint = fingerprint64(fingerPrint, entry.first);
        fingerPrint = fingerprint64(fingerPrint, (int)descriptor.kind);
    }

    int64_t signed_fp{};

    std::memcpy(&signed_fp, &fingerPrint, sizeof(uint64_t));
    return signed_fp;
}

bool
kind_size_comparator(const field_descriptor* i, const field_descriptor* j)
{
    auto i_kind_size = field_operations::get(i->kind).kind_size_in_byte_func();
    auto j_kind_size = field_operations::get(j->kind).kind_size_in_byte_func();
    return i_kind_size > j_kind_size;
}

schema::schema(
  std::string type_name,
  std::unordered_map<std::string, field_descriptor>&& field_definition_map)
  : type_name_(std::move(type_name))
  , field_definition_map_(std::move(field_definition_map))
{
    std::vector<field_descriptor*> fixed_size_fields;
    std::vector<field_descriptor*> boolean_fields;
    std::vector<field_descriptor*> variable_size_fields;

    std::map<std::string, field_descriptor> sorted_fields(
      field_definition_map_.begin(), field_definition_map_.end());
    for (auto& item : sorted_fields) {
        field_descriptor& descriptor = item.second;
        field_kind kind = descriptor.kind;
        if (field_operations::get(kind).kind_size_in_byte_func() ==
            field_kind_based_operations::VARIABLE_SIZE) {
            variable_size_fields.push_back(&descriptor);
        } else if (kind == field_kind::BOOLEAN) {
            boolean_fields.push_back(&descriptor);
        } else {
            fixed_size_fields.push_back(&descriptor);
        }
    }

    std::sort(
      fixed_size_fields.begin(), fixed_size_fields.end(), kind_size_comparator);

    int offset = 0;
    for (auto descriptor : fixed_size_fields) {
        descriptor->offset = offset;
        offset +=
          field_operations::get(descriptor->kind).kind_size_in_byte_func();
    }

    int8_t bit_offset = 0;
    for (auto descriptor : boolean_fields) {
        descriptor->offset = offset;
        descriptor->bit_offset =
          static_cast<int8_t>(bit_offset % util::Bits::BITS_IN_BYTE);
        bit_offset++;
        if (bit_offset % util::Bits::BITS_IN_BYTE == 0) {
            offset += 1;
        }
    }
    if (bit_offset % util::Bits::BITS_IN_BYTE != 0) {
        offset += 1;
    }

    fixed_size_fields_length_ = offset;

    int index = 0;
    for (auto descriptor : variable_size_fields) {
        descriptor->index = index;
        index++;
    }

    for (auto& item : sorted_fields) {
        auto field = field_definition_map_.find(item.first);

        assert(field != end(field_definition_map_));
        field->second = item.second;
    }

    number_of_var_size_fields_ = index;
    schema_id_ = rabin_finger_print::fingerprint64(type_name_, sorted_fields);
}

int64_t
schema::schema_id() const
{
    return schema_id_;
}

size_t
schema::number_of_var_size_fields() const
{
    return number_of_var_size_fields_;
}

size_t
schema::fixed_size_fields_length() const
{
    return fixed_size_fields_length_;
}

const std::string&
schema::type_name() const
{
    return type_name_;
}

const std::unordered_map<std::string, field_descriptor>&
schema::fields() const
{
    return field_definition_map_;
}

boost::optional<field_descriptor>
schema::get_field(const std::string& field_name) const
{
    auto descriptor_itr = field_definition_map_.find(field_name);

    if (descriptor_itr == end(field_definition_map_))
        return boost::none;

    return descriptor_itr->second;
}

bool
operator==(const schema& x, const schema& y)
{
    return x.number_of_var_size_fields() == y.number_of_var_size_fields() &&
           x.fixed_size_fields_length() == y.fixed_size_fields_length() &&
           x.schema_id() == y.schema_id() && x.type_name() == y.type_name() &&
           x.fields() == y.fields();
}

bool
operator!=(const schema& x, const schema& y)
{
    return !(x == y);
}

std::ostream&
operator<<(std::ostream& os, const schema& schema)
{
    os << "Schema { className = " << schema.type_name()
       << ", numberOfComplextFields = " << schema.number_of_var_size_fields()
       << ",primitivesLength = " << schema.fixed_size_fields_length()
       << ",fields {";
    for (const auto& item : schema.fields()) {
        os << item.first << " " << item.second << ",";
    }
    os << "}";
    return os;
}

} // namespace pimpl

namespace pimpl {

exception::hazelcast_serialization
compact_util::exception_for_unexpected_null_value(
  const std::string& field_name,
  const std::string& method_prefix,
  const std::string& method_suffix)
{
    return exception::hazelcast_serialization{ boost::str(
      boost::format("Error while reading %1%. null value can not be read via "
                    "%2%%3% methods. Use %2%Nullable%3% instead.") %
      field_name % method_prefix % method_suffix) };
}

exception::hazelcast_serialization
compact_util::exception_for_unexpected_null_value_in_array(
  const std::string& field_name,
  const std::string& method_prefix,
  const std::string& method_suffix)
{
    return exception::hazelcast_serialization{ boost::str(
      boost::format(
        "Error while reading %1%. null value can not be read via %2%ArrayOf%3% "
        "methods. Use %2%ArrayOfNullable%3% instead.") %
      field_name % method_prefix % method_suffix) };
}

schema_writer::schema_writer(std::string type_name)
  : type_name_(std::move(type_name))
{}

void
schema_writer::add_field(std::string field_name, enum field_kind kind)
{
    if (field_definition_map_.find(field_name) != end(field_definition_map_)) {
        BOOST_THROW_EXCEPTION(exception::hazelcast_serialization{
          "Field with the name '" + field_name + "' already exists." });
    }

    field_definition_map_.emplace(move(field_name), field_descriptor{ kind });
}

schema
schema_writer::build() &&
{
    return schema{ type_name_, std::move(field_definition_map_) };
}

default_schema_service::default_schema_service(spi::ClientContext& context)
  : retry_pause_millis_{ context.get_client_properties().get_integer(
      context.get_client_properties().get_invocation_retry_pause_millis()) }
  , max_put_retry_count_{ context.get_client_properties().get_integer(
      client_property{ MAX_PUT_RETRY_COUNT, MAX_PUT_RETRY_COUNT_DEFAULT }) }
  , context_{ context }
{
}

std::shared_ptr<schema>
default_schema_service::get(int64_t schemaId)
{
    auto ptr = replicateds_.get(schemaId);

    if (ptr) {
        return ptr;
    }

    auto logger = context_.get_logger();
    if (logger.enabled(logger::level::finest)) {
        logger.log(
          logger::level::finest,
          boost::str(boost::format("Could not find schema id %1% locally, will "
                                   "search on the cluster %1%") %
                     schemaId));
    }

    using namespace protocol::codec;

    auto request_message = client_fetchschema_encode(schemaId);

    auto invocation = spi::impl::ClientInvocation::create(
      context_, request_message, SERVICE_NAME);
    auto message = invocation->invoke().get();

    message.skip_frame();
    auto sch = message.get_nullable<schema>();

    std::shared_ptr<schema> schema_ptr;

    if (sch) {
        schema_ptr = std::make_shared<schema>(std::move(*sch));
        replicateds_.put_if_absent(schemaId, schema_ptr);
    }

    return schema_ptr;
}

void
default_schema_service::replicate_schema_in_cluster(schema s)
{
    using hazelcast::client::protocol::ClientMessage;
    using namespace protocol::codec;

    for (int i = 0; i < max_put_retry_count_; ++i) {
        auto message = client_sendschema_encode(s);

        auto invocation =
          spi::impl::ClientInvocation::create(context_, message, SERVICE_NAME);

        auto response = invocation->invoke().get();
        auto replicated_member_uuids = send_schema_response_decode(response);
        auto members = context_.get_cluster().get_members();

        bool contains;
        for (const member& member : members) {

            contains = replicated_member_uuids.find(member.get_uuid()) !=
                       end(replicated_member_uuids);

            if (!contains) {
                if (i == (max_put_retry_count_ - 1)) {
                    throw exception::illegal_state{
                        "default_schema_service::replicate_schema_attempt",
                        (boost::format("The schema %1% cannot be "
                                       "replicated in the cluster, after "
                                       "%2% retries. It might be the case "
                                       "that the client is experiencing a "
                                       "split-brain, and continue putting "
                                       "the data associated with that "
                                       "schema might result in data loss. "
                                       "It might be possible to replicate "
                                       "the schema after some time, when "
                                       "the cluster is healed.") %
                         s % max_put_retry_count_)
                          .str()
                    };
                } else {
                    std::this_thread::sleep_for(
                      std::chrono::milliseconds{ retry_pause_millis_ });

                    if (!context_.get_lifecycle_service().is_running()) {
                        return;
                    }

                    break;
                }
            }
        }

        if (contains) {
            put_if_absent(std::move(s));

            break;
        }
    }
}

bool
default_schema_service::is_schema_replicated(const schema& s)
{
    return bool(replicateds_.get(s.schema_id()));
}

void
default_schema_service::put_if_absent(schema s)
{
    auto s_p = std::make_shared<schema>(std::move(s));
    auto existing = replicateds_.put_if_absent(s_p->schema_id(), s_p);

    if (!existing) {
        return;
    }

    if (*s_p != *existing) {
        throw exception::illegal_state{
            "default_schema_service::replicate_schema_attempt",
            (boost::format("Schema with schemaId %1% "
                           "already exists. Existing "
                           "schema %2%, new schema %3%") %
             s_p->schema_id() % *existing % *s_p)
              .str()
        };
    }
}

/**
 * Decodes response of send schema request
 */
std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>>
default_schema_service::send_schema_response_decode(
  protocol::ClientMessage& message)
{
    message.skip_frame();
    return message.get<std::unordered_set<boost::uuids::uuid,
                                          boost::hash<boost::uuids::uuid>>>();
}

bool
default_schema_service::has_any_schemas() const
{
    return replicateds_.size();
}

std::ostream&
operator<<(std::ostream& os, const std::vector<schema>& schemas)
{
    os << "Schemas {";

    for (const auto& s : schemas)
        os << s << " , ";

    os << "}";

    return os;
}

void
default_schema_service::replicate_all_schemas()
{
    using level = hazelcast::logger::level;
    using namespace protocol::codec;

    auto logger = context_.get_logger();
    if (replicateds_.empty()) {
        if (logger.enabled(level::finest)) {
            logger.log(level::finest,
                       "There is no schema to send to the cluster.");
        }

        return;
    }

    std::vector<std::shared_ptr<schema>> schemas_sptr = replicateds_.values();
    std::vector<schema> all_schemas;

    all_schemas.reserve(schemas_sptr.size());

    transform(begin(schemas_sptr),
              end(schemas_sptr),
              back_inserter(all_schemas),
              [](const std::shared_ptr<schema>& s) { return *s; });

    if (logger.enabled(level::finest)) {
        logger.log(
          level::finest,
          (boost::format("Sending schemas to the cluster %1%") % all_schemas)
            .str());
    }

    auto message = client_sendallschemas_encode(all_schemas);

    auto invocation =
      spi::impl::ClientInvocation::create(context_, message, SERVICE_NAME);

    invocation->invoke_urgent().get();
}

compact_stream_serializer::compact_stream_serializer(
  default_schema_service& service)
  : schema_service{ service }
{
}

generic_record::generic_record
compact_stream_serializer::read_generic_record(object_data_input& in)
{
    int64_t schema_id = in.read<int64_t>();

    auto sch = schema_service.get(schema_id);

    if (!sch) {
        throw exception::hazelcast_serialization{
            "compact_stream_serializer::read_generic_record",
            boost::str(
              boost::format(
                "The schema can not be found with id %1%") %
              schema_id)
        };
    }

    compact::compact_reader reader = create_compact_reader(*this, in, *sch);
    generic_record::generic_record_builder builder{ sch };

    for (const std::pair<const std::string, field_descriptor>& p : sch->fields()) {
        const std::string& field_name = p.first;
        const field_descriptor& descriptor = p.second;
        field_operations::get(descriptor.kind)
          .read_generic_record_or_primitive(reader, builder, field_name);
    }

    return builder.build();
}

void
compact_stream_serializer::write_generic_record(
  const generic_record::generic_record& record,
  object_data_output& out)
{
    const schema& s = record.get_schema();

    if (!schema_service.is_schema_replicated(s)) {
        out.schemas_will_be_replicated_.push_back(s);
    }

    out.write<int64_t>(s.schema_id());
    default_compact_writer default_writer(*this, out, s);

    const auto& fields = s.fields();
    for (std::pair<std::string, field_descriptor> pair : fields) {
        const std::string& field = pair.first;
        const field_descriptor& desc = pair.second;

        field_operations::get(desc.kind).write_field_from_record_to_writer(
          default_writer, record, field);
    }

    default_writer.end();
}

field_kind_based_operations::field_kind_based_operations()
  : kind_size_in_byte_func(DEFAULT_KIND_SIZE_IN_BYTES)
{}

field_kind_based_operations::field_kind_based_operations(
  kind_size_in_bytes_fn kind_size_fn,
  write_field_from_record_to_writer_fn write_fn,
  read_generic_record_or_primitive_fn read_fn,
  write_json_formatted_field_fn json_fn)
  : kind_size_in_byte_func(std::move(kind_size_fn))
  , write_field_from_record_to_writer(std::move(write_fn))
  , read_generic_record_or_primitive(std::move(read_fn))
  , write_json_formatted_field(std::move(json_fn))
{}

const field_kind_based_operations&
field_operations::get(field_kind kind)
{
    using util::Bits;
    using namespace boost::property_tree;

    static const char* NULL_STRING = "null";
    static const char* BOOL_STRING[2] = { "true", "false" };

    static auto time_to_string = [](const local_time& lt) {
        return boost::str(boost::format("%02d:%02d:%02d.%d") % int(lt.hours) %
                          int(lt.minutes) % int(lt.seconds) % lt.nanos);
    };

    static auto date_to_string = [](const local_date& lt) {
        return boost::str(boost::format("%d-%02d-%02d") % lt.year %
                          int(lt.month) % int(lt.day_of_month));
    };

    static auto timestamp_to_string = [](const local_date_time& lt) {
        return boost::str(boost::format("%1%T%2%") % date_to_string(lt.date) %
                          time_to_string(lt.time));
    };

    static auto timestamp_with_timezone_to_string =
      [](const offset_date_time& lt) {
          auto hours = lt.zone_offset_in_seconds / 60;
          auto seconds = lt.zone_offset_in_seconds % 60;
          return boost::str(boost::format("%1%%2%:%3%") %
                            timestamp_to_string(lt.date_time) % hours %
                            seconds);
      };

    static const field_kind_based_operations ALL[] = {
        field_kind_based_operations{}, // [0]NOT_AVAILABLE
        field_kind_based_operations{
          // [1]BOOLEAN
          [] { return 0; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_boolean(field, record.get_boolean(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_boolean(field, reader.read_boolean(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              parent.put(field_name, record.get_boolean(field_name));
          } },
        field_kind_based_operations{
          // [2]ARRAY_OF_BOOLEAN
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_boolean(field,
                                            record.get_array_of_boolean(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_boolean(field,
                                           reader.read_array_of_boolean(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_boolean(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (bool value : *values) {
                      array.push_back(
                        ptree::value_type("", BOOL_STRING[value]));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [3]INT8
          [] { return 1; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_int8(field, record.get_int8(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_int8(field, reader.read_int8(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              parent.put(field_name, record.get_int8(field_name));
          } },
        field_kind_based_operations{
          // [4]ARRAY_OF_INT8
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_int8(field,
                                         record.get_array_of_int8(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_int8(field,
                                        reader.read_array_of_int8(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_int8(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (auto value : *values) {
                      array.push_back(
                        ptree::value_type("", std::to_string(value)));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{}, // [5]
        field_kind_based_operations{}, // [6]
        field_kind_based_operations{
          // [7]INT16
          [] { return Bits::SHORT_SIZE_IN_BYTES; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_int16(field, record.get_int16(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_int16(field, reader.read_int16(field));
          },
          [](boost::property_tree::ptree& pt,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              pt.put(field_name, record.get_int16(field_name));
          } },
        field_kind_based_operations{
          // [8]ARRAY_OF_INT16
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_int16(field,
                                          record.get_array_of_int16(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_int16(field,
                                         reader.read_array_of_int16(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_int16(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (auto value : *values) {
                      array.push_back(
                        ptree::value_type("", std::to_string(value)));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [9]INT32
          [] { return Bits::INT_SIZE_IN_BYTES; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_int32(field, record.get_int32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_int32(field, reader.read_int32(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              parent.put(field_name, record.get_int32(field_name));
          } },
        field_kind_based_operations{
          // [10]ARRAY_OF_INT32
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_int32(field,
                                          record.get_array_of_int32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_int32(field,
                                         reader.read_array_of_int32(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_int32(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (auto value : *values) {
                      array.push_back(
                        ptree::value_type("", std::to_string(value)));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [11]INT64
          [] { return Bits::LONG_SIZE_IN_BYTES; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_int64(field, record.get_int64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_int64(field, reader.read_int64(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              parent.put(field_name, record.get_int64(field_name));
          } },
        field_kind_based_operations{
          // [12]ARRAY_OF_INT64
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_int64(field,
                                          record.get_array_of_int64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_int64(field,
                                         reader.read_array_of_int64(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_int64(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (auto value : *values) {
                      array.push_back(
                        ptree::value_type("", std::to_string(value)));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [13]FLOAT32
          [] { return Bits::FLOAT_SIZE_IN_BYTES; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_float32(field, record.get_float32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_float32(field, reader.read_float32(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              parent.put(field_name, record.get_float32(field_name));
          } },
        field_kind_based_operations{
          // [14]ARRAY_OF_FLOAT32
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_float32(field,
                                            record.get_array_of_float32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_float32(field,
                                           reader.read_array_of_float32(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_float32(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (auto value : *values) {
                      array.push_back(
                        ptree::value_type("", std::to_string(value)));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [15]FLOAT64
          [] { return Bits::DOUBLE_SIZE_IN_BYTES; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_float64(field, record.get_float64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_float64(field, reader.read_float64(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              parent.put(field_name, record.get_float64(field_name));
          } },
        field_kind_based_operations{
          // [16]ARRAY_OF_FLOAT64
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_float64(field,
                                            record.get_array_of_float64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_float64(field,
                                           reader.read_array_of_float64(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_float64(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (auto value : *values) {
                      array.push_back(
                        ptree::value_type("", std::to_string(value)));
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [17]STRING
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_string(field, record.get_string(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_string(field, reader.read_string(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_string(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, *value);
              }
          } },
        field_kind_based_operations{
          // [18]ARRAY_OF_STRING
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_string(field,
                                           record.get_array_of_string(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_string(field,
                                          reader.read_array_of_string(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_string(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(ptree::value_type("", *value));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [19]DECIMAL
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_decimal(field, record.get_decimal(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_decimal(field, reader.read_decimal(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_decimal(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name,
                             value->unscaled.str() + "E" +
                               std::to_string(value->scale));
              }
          } },
        field_kind_based_operations{
          // [20]ARRAY_OF_DECIMAL
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_decimal(field,
                                            record.get_array_of_decimal(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_decimal(field,
                                           reader.read_array_of_decimal(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_decimal(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("",
                                              value->unscaled.str() + "E" +
                                                std::to_string(value->scale)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [21]TIME
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_time(field, record.get_time(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_time(field, reader.read_time(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_time(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, time_to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [22]ARRAY_OF_TIME
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_time(field,
                                         record.get_array_of_time(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_time(field,
                                        reader.read_array_of_time(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_time(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", time_to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [23]DATE
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_date(field, record.get_date(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_date(field, reader.read_date(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_date(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, date_to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [24]ARRAY_OF_DATE
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_date(field,
                                         record.get_array_of_date(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_date(field,
                                        reader.read_array_of_date(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_date(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", date_to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [25]TIMESTAMP
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_timestamp(field, record.get_timestamp(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_timestamp(field, reader.read_timestamp(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_timestamp(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, timestamp_to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [26]ARRAY_OF_TIMESTAMP
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_timestamp(
                field, record.get_array_of_timestamp(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_timestamp(
                field, reader.read_array_of_timestamp(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values = record.get_array_of_timestamp(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", timestamp_to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [27]TIMESTAMP_WITH_TIMEZONE
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_timestamp_with_timezone(
                field, record.get_timestamp_with_timezone(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_timestamp_with_timezone(
                field, reader.read_timestamp_with_timezone(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value =
                record.get_timestamp_with_timezone(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name,
                             timestamp_with_timezone_to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [28]ARRAY_OF_TIMESTAMP_WITH_TIMEZONE
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_timestamp_with_timezone(
                field, record.get_array_of_timestamp_with_timezone(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_timestamp_with_timezone(
                field, reader.read_array_of_timestamp_with_timezone(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_timestamp_with_timezone(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(ptree::value_type(
                            "", timestamp_with_timezone_to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [29]COMPACT
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_compact(field, record.get_generic_record(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_generic_record(
                field,
                reader.read_compact<generic_record::generic_record>(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_generic_record(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put_child(field_name, write_generic_record(*value));
              }
          } },
        field_kind_based_operations{
          // [30]ARRAY_OF_COMPACT
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_compact(
                field, record.get_array_of_generic_record(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_generic_record(
                field,
                reader.read_array_of_compact<generic_record::generic_record>(
                  field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_generic_record(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(ptree::value_type(
                            "", write_generic_record(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{}, // [31]
        field_kind_based_operations{}, // [32]
        field_kind_based_operations{
          // [33]NULLABLE_BOOLEAN
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_boolean(field,
                                            record.get_nullable_boolean(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_boolean(field,
                                           reader.read_nullable_boolean(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_boolean(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, *value);
              }
          } },
        field_kind_based_operations{
          // [34]ARRAY_OF_NULLABLE_BOOLEAN
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_boolean(
                field, record.get_array_of_nullable_boolean(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_boolean(
                field, reader.read_array_of_nullable_boolean(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_boolean(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", BOOL_STRING[*value]));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [35]NULLABLE_INT8
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_int8(field,
                                         record.get_nullable_int8(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_int8(field,
                                        reader.read_nullable_int8(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_int8(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, std::to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [36]ARRAY_OF_NULLABLE_INT8
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_int8(
                field, record.get_array_of_nullable_int8(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_int8(
                field, reader.read_array_of_nullable_int8(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_int8(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", std::to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [37]NULLABLE_INT16
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_int16(field,
                                          record.get_nullable_int16(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_int16(field,
                                         reader.read_nullable_int16(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_int16(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, std::to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [38]ARRAY_OF_NULLABLE_INT16
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_int16(
                field, record.get_array_of_nullable_int16(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_int16(
                field, reader.read_array_of_nullable_int16(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_int16(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", std::to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [39]NULLABLE_INT32
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_int32(field,
                                          record.get_nullable_int32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_int32(field,
                                         reader.read_nullable_int32(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_int32(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, std::to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [40]ARRAY_OF_NULLABLE_INT32
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_int32(
                field, record.get_array_of_nullable_int32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_int32(
                field, reader.read_array_of_nullable_int32(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_int32(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", std::to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [41]NULLABLE_INT64
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_int64(field,
                                          record.get_nullable_int64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_int64(field,
                                         reader.read_nullable_int64(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_int64(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, std::to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [42]ARRAY_OF_NULLABLE_INT64
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_int64(
                field, record.get_array_of_nullable_int64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_int64(
                field, reader.read_array_of_nullable_int64(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_int64(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", std::to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [43]NULLABLE_FLOAT32
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_float32(field,
                                            record.get_nullable_float32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_float32(field,
                                           reader.read_nullable_float32(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_float32(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, std::to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [44]ARRAY_OF_NULLABLE_FLOAT32
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_float32(
                field, record.get_array_of_nullable_float32(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_float32(
                field, reader.read_array_of_nullable_float32(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_float32(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", std::to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } },
        field_kind_based_operations{
          // [45]NULLABLE_FLOAT64
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_nullable_float64(field,
                                            record.get_nullable_float64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_nullable_float64(field,
                                           reader.read_nullable_float64(field));
          },
          [](boost::property_tree::ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& value = record.get_nullable_float64(field_name);

              if (!value) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  parent.put(field_name, std::to_string(*value));
              }
          } },
        field_kind_based_operations{
          // [46]ARRAY_OF_NULLABLE_FLOAT64
          [] { return field_kind_based_operations::VARIABLE_SIZE; },
          [](default_compact_writer& writer,
             const generic_record::generic_record& record,
             const std::string& field) {
              writer.write_array_of_nullable_float64(
                field, record.get_array_of_nullable_float64(field));
          },
          [](compact::compact_reader& reader,
             generic_record::generic_record_builder& builder,
             const std::string& field) {
              builder.set_array_of_nullable_float64(
                field, reader.read_array_of_nullable_float64(field));
          },
          [](ptree& parent,
             const generic_record::generic_record& record,
             const std::string& field_name) {
              const auto& values =
                record.get_array_of_nullable_float64(field_name);

              if (!values) {
                  parent.put(field_name, NULL_STRING);
              } else {
                  ptree array;

                  for (const auto& value : *values) {
                      if (!value) {
                          array.push_back(ptree::value_type("", NULL_STRING));
                      } else {
                          array.push_back(
                            ptree::value_type("", std::to_string(*value)));
                      }
                  }

                  parent.put_child(field_name, array);
              }
          } }
    };

    return ALL[std::size_t(kind)];
}

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast
