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

//#include <unordered_map>
//#include <unordered_set>
//#include <type_traits>
//
//#include <boost/any.hpp>
//#include <boost/optional.hpp>
//#include <boost/optional/optional_io.hpp>
//#include <boost/uuid/uuid.hpp>
#include <boost/thread/future.hpp>
#include "hazelcast/util/export.h"
//#include "hazelcast/client/hazelcast_json_value.h"
//#include "hazelcast/client/serialization/pimpl/data_input.h"
//#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/serialization/serialization.h"
//#include "hazelcast/client/serialization_config.h"
#include "hazelcast/util/SynchronizedMap.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
class hazelcast_client;

namespace serialization {

/**
 *      static std::string get_type_name();
 *      static void write_compact(const T& object, compact_writer &out);
 *      static T read_compact(compact_reader &in);
 */
struct compact_serializer
{};

namespace pimpl {
// forward declarations

enum HAZELCAST_API field_kind
{
    INT32 = 8,
    STRING = 16
};

class HAZELCAST_API field_descriptor
{
public:
    void index(int32_t index) { this->index_ = index; }

    void offset(int32_t offset) { this->offset_ = offset; }

    void bit_offset(int8_t bit_offset) { this->bit_offset_ = bit_offset; }

    enum field_kind field_kind() { return field_kind_; }

    std::string field_name() { return field_name_; }

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

class HAZELCAST_API schema
{
public:
    long schema_id() const;

private:
    long schema_id_;
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
    boost::future<void> put(std::shared_ptr<schema> schema_ptr);

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

} // namespace pimpl

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
};

} // namespace serialization
} // namespace client
} // namespace hazelcast