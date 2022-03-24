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
#pragma once

#include <boost/thread/future.hpp>
#include <utility>
#include "hazelcast/util/export.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/SynchronizedMap.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace serialization {
namespace pimpl {
class default_compact_writer;
class schema_writer;
} // namespace pimpl

/**
 *      static std::string get_type_name();
 *      static void write_compact(const T& object, compact_writer &out);
 *      static T read_compact(compact_reader &in);
 */
struct compact_serializer
{};

/**
 * Provides means of reading compact serialized fields from the binary data.
 * <p>
 * Read operations might throw {@link HazelcastSerializationException}
 * when a field with the given name is not found or there is a type mismatch. On
 * such occasions, one might provide default values to the read methods to
 * return it in case of the failure scenarios described above. Providing default
 * values might be especially useful, if the class might evolve in future,
 * either by adding or removing fields.
 *
 * @since 5.2
 */
class HAZELCAST_API compact_reader
{
public:
    /**
     * Reads a 32-bit two's complement signed integer.
     *
     * @param fieldName name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in
     * the schema or the type of the field does not match with the one defined
     * in the schema.
     */
    int32_t read_int32(const std::string& field_name);

    /**
     * Reads a 32-bit two's complement signed integer or returns the default
     * value.
     *
     * @param fieldName    name of the field.
     * @param defaultValue default value to return if the field with the given
     * name does not exist in the schema or the type of the field does not match
     * with the one defined in the schema.
     * @return the value or the default value of the field.
     */
    int32_t read_int32(const std::string& field_name, int32_t default_value);

    /**
     * Reads an UTF-8 encoded string.
     *
     * @param fieldName name of the field.
     * @return the value of the field.
     * @throws hazelcast_serialization if the field does not exist in
     * the schema or the type of the field does not match with the one defined
     * in the schema.
     */
    boost::optional<std::string> read_string(const std::string& field_name);

    /**
     * Reads an UTF-8 encoded string or returns the default value.
     *
     * @param fieldName    name of the field.
     * @param defaultValue default value to return if the field with the given
     * name does not exist in the schema or the type of the field does not match
     * with the one defined in the schema.
     * @return the value or the default value of the field.
     */
    boost::optional<std::string> read_string(
      const std::string& field_name,
      const boost::optional<std::string>& default_value);
};

/**
 *  Provides means of writing compact serialized fields to the binary data.
 */
class HAZELCAST_API compact_writer
{
public:
    /**
     * Writes a 32-bit two's complement signed integer.
     *
     * @param fieldName name of the field.
     * @param value     to be written.
     */
    void write_int32(const std::string& field_name, int32_t value);

    /**
     * Writes an UTF-8 encoded string.
     *
     * @param fieldName name of the field.
     * @param value     to be written.
     */
    void write_string(const std::string& field_name,
                      const boost::optional<std::string>& value);

protected:
    explicit compact_writer(
      pimpl::default_compact_writer* default_compact_writer);
    explicit compact_writer(pimpl::schema_writer* schema_writer);

private:
    pimpl::default_compact_writer* default_compact_writer;
    pimpl::schema_writer* schema_writer;
};

namespace pimpl {

class HAZELCAST_API default_compact_writer
{
public:
    void write_int32(const std::string& field_name, int32_t value);
    void write_string(const std::string& field_name,
                      const boost::optional<std::string>& value);
};

enum HAZELCAST_API field_kind
{
    BOOLEAN = 0,
    ARRAY_OF_BOOLEAN = 1,
    INT32 = 8,
    STRING = 16,
    TIMESTAMP_WITH_TIMEZONE = 26,
    ARRAY_OF_NULLABLE_FLOAT64 = 45
};
static const int NUMBER_OF_FIELD_KINDS = ARRAY_OF_NULLABLE_FLOAT64 + 1;

class HAZELCAST_API field_descriptor
{
public:
    field_descriptor(std::string field_name, enum field_kind field_kind)
      : field_name_(std::move(field_name))
      , field_kind_(field_kind)
    {}

    void index(int32_t index) { this->index_ = index; }

    void offset(int32_t offset) { this->offset_ = offset; }

    void bit_offset(int8_t bit_offset) { this->bit_offset_ = bit_offset; }

    enum field_kind field_kind() const { return field_kind_; }

    const std::string& field_name() const { return field_name_; }

    /**
     * @return the index of offset of the non-primitive field. if field is
     * primitive returns -1
     */
    int32_t index() const { return index_; }

    /**
     * @return the offset to read  the primitive field from. If field is not
     * primitive returns -1
     */
    int32_t offset() const { return offset_; }

    /**
     * Applicable only for boolean field
     *
     * @return the offset of the boolean within the given byte via `offset`
     */
    int8_t bit_offset() const { return bit_offset_; }

private:
    std::string field_name_;
    enum field_kind field_kind_;
    int32_t index_ = -1;
    int32_t offset_ = -1;
    int8_t bit_offset_ = -1;
};
constexpr int SCHEMA_DS_FACTORY_ID = -42;
constexpr int SCHEMA_CLASS_ID = 1;
constexpr int SEND_SCHEMA_OPERATION = 2;
constexpr int FETCH_SCHEMA_OPERATION = 3;
constexpr int SEND_ALL_SCHEMAS_OPERATION = 4;
class HAZELCAST_API schema
{
public:
    schema() = default;
    schema(std::string type_name,
           std::map<std::string, field_descriptor>&& field_definition_map);
    long schema_id() const;
    const field_descriptor& get_field(const std::string& field_name) const;
    size_t number_of_var_size_fields() const;
    size_t fixed_size_fields_length() const;
    const std::string& type_name() const;
    size_t field_count() const;
    const std::map<std::string, field_descriptor>& fields() const;

private:
    std::string type_name_;
    std::map<std::string, field_descriptor> field_definition_map_;
    size_t number_of_var_size_fields_{};
    size_t fixed_size_fields_length_{};
    long schema_id_{};
};
} // namespace pimpl
template<>
struct hz_serializer<pimpl::schema> : public identified_data_serializer
{
    static int32_t get_factory_id();
    static int32_t get_class_id();
    static void write_data(const pimpl::schema& object,
                           object_data_output& out);
    static pimpl::schema read_data(object_data_input& in);
};
namespace pimpl {
class HAZELCAST_API schema_writer
{
public:
    explicit schema_writer(std::string type_name);
    void add_field(const std::string& field_name, enum field_kind kind);
    schema build() &&;

private:
    std::map<std::string, field_descriptor> field_definition_map;
    std::string type_name;
};

/**
 * Service to put and get metadata to cluster.
 */
class HAZELCAST_API default_schema_service
{
public:
    /**
     * Gets the schema with the given id either by
     * <ul>
     *     <li>returning it directly from the local registry, if it exists.</li>
     *     <li>searching the cluster.</li>
     * </ul>
     */
    boost::future<schema> get(long schemaId);

    /**
     * Puts the schema with the given id to the cluster.
     */
    boost::future<void> put(const std::shared_ptr<schema>& schema_ptr);

private:
    util::SynchronizedMap<long, schema> schemas;
};

class HAZELCAST_API CompactSerializer
{
public:
    CompactSerializer();

    template<typename T>
    T read(object_data_input& in);

    template<typename T>
    void write(const T& object, object_data_output& out);

private:
    default_schema_service schema_service;
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

namespace field_operations {
static field_kind_based_operations
get(int index);
}

} // namespace pimpl

} // namespace serialization

} // namespace client
} // namespace hazelcast