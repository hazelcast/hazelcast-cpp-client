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

#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include <boost/any.hpp>
#include <boost/optional.hpp>
#include<boost/optional/optional_io.hpp>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/PartitionAware.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Disposable.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace serialization {
            namespace pimpl {
                // forward declarations
                class PortableContext;

                class ClassDefinitionContext;

                class ClassDefinitionWriter;

                class DefaultPortableWriter;

                class DefaultPortableReader;

                class MorphingPortableReader;

                class PortableSerializer;

                class DataSerializer;

                class SerializationService;

                enum struct HAZELCAST_API serialization_constants {
                    CONSTANT_TYPE_NULL = 0,
                    CONSTANT_TYPE_PORTABLE = -1,
                    CONSTANT_TYPE_DATA = -2,
                    CONSTANT_TYPE_BYTE = -3,
                    CONSTANT_TYPE_BOOLEAN = -4,
                    CONSTANT_TYPE_CHAR = -5,
                    CONSTANT_TYPE_SHORT = -6,
                    CONSTANT_TYPE_INTEGER = -7,
                    CONSTANT_TYPE_LONG = -8,
                    CONSTANT_TYPE_FLOAT = -9,
                    CONSTANT_TYPE_DOUBLE = -10,
                    CONSTANT_TYPE_STRING = -11,
                    CONSTANT_TYPE_BYTE_ARRAY = -12,
                    CONSTANT_TYPE_BOOLEAN_ARRAY = -13,
                    CONSTANT_TYPE_CHAR_ARRAY = -14,
                    CONSTANT_TYPE_SHORT_ARRAY = -15,
                    CONSTANT_TYPE_INTEGER_ARRAY = -16,
                    CONSTANT_TYPE_LONG_ARRAY = -17,
                    CONSTANT_TYPE_FLOAT_ARRAY = -18,
                    CONSTANT_TYPE_DOUBLE_ARRAY = -19,
                    CONSTANT_TYPE_STRING_ARRAY = -20,
                    CONSTANT_TYPE_UUID = -21,
                    JAVASCRIPT_JSON_SERIALIZATION_TYPE = -130,

                    CONSTANT_TYPE_GLOBAL = INT32_MIN
                    // ------------------------------------------------------------
                };

                /**
                 * This class represents the type of a Hazelcast serializable object. The fields can take the following
                 * values:
                 * 1. Primitive types: factoryId=-1, classId=-1, typeId is the type id for that primitive as listed in
                 * @link SerializationConstants
                 * 2. Array of primitives: factoryId=-1, classId=-1, typeId is the type id for that array as listed in
                 * @link SerializationConstants
                 * 3. identified_data_serializer: factory, class and type ids are non-negative values as registered by
                 * the DataSerializableFactory.
                 * 4. portable_serializer: factory, class and type ids are non-negative values.
                 * 5. Custom serialized objects: factoryId=-1, classId=-1, typeId is the non-negative type id as
                 * registered by custom_serializer.
                 *
                 */
                struct HAZELCAST_API ObjectType {
                    ObjectType();

                    serialization_constants type_id;
                    int32_t factory_id;
                    int32_t class_id;

                };

                std::ostream HAZELCAST_API &operator<<(std::ostream &os, const ObjectType &type);
            }
        }

        /**
         * typed_data class is a wrapper class for the serialized binary data. It does late deserialization of the data
         * only when the get method is called.
         */
        class HAZELCAST_API typed_data {
        public:
            typed_data();

            typed_data(serialization::pimpl::data d,
                       serialization::pimpl::SerializationService &serialization_service);

            /**
             *
             * @return The type of the underlying object for this binary.
             */
            serialization::pimpl::ObjectType get_type() const;

            /**
             * Deserializes the underlying binary data and produces the object of type T.
             *
             * <b>CAUTION</b>: The type that you provide should be compatible with what object type is returned with
             * the get_type API, otherwise you will either get an exception of incorrectly try deserialize the binary data.
             *
             * @tparam T The type to be used for deserialization
             * @return The object instance of type T.
             */
            template <typename T>
            boost::optional<T> get() const;

            /**
             * Internal API
             * @return The pointer to the internal binary data.
             */
            const serialization::pimpl::data &get_data() const;

        private:
            serialization::pimpl::data data_;
            serialization::pimpl::SerializationService *ss_;
        };

        bool HAZELCAST_API operator<(const typed_data &lhs, const typed_data &rhs);

        namespace serialization {
            class ObjectDataInput;
            class ObjectDataOutput;
            class PortableReader;

            namespace pimpl {
                // forward declarations
                class PortableContext;
                class ClassDefinitionContext;
                class ClassDefinitionWriter;
                class DefaultPortableWriter;
                class DefaultPortableReader;
                class MorphingPortableReader;
                class PortableSerializer;
                class DataSerializer;
                class SerializationService;
            }

            template<typename T>
            struct hz_serializer {};

            struct builtin_serializer {};

            struct custom_serializer {};

            struct global_serializer {
                virtual ~global_serializer() = default;

                static pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_GLOBAL;
                }

                virtual void write(const boost::any &object, ObjectDataOutput &out) = 0;

                virtual boost::any read(ObjectDataInput &in) = 0;
            };

            /**
             * Classes derived from this class should implement the following static methods:
             *      static int32_t getClassId() noexcept;
             *      static int32_t getFactoryId() noexcept;
             *      static int32_t writeData(const T &object, ObjectDataOutput &out);
             *      static T readData(ObjectDataInput &in);
             */
            struct identified_data_serializer {
            };

            /**
             * Classes derived from this class should implement the following static methods:
             *      static int32_t getClassId() noexcept;
             *      static int32_t getFactoryId() noexcept;
             *      static int32_t write_portable(const T &object, PortableWriter &out);
             *      static T read_portable(PortableReader &in);
             */
            struct portable_serializer {
            };

            struct versioned_portable_serializer : public portable_serializer {
            };

            template<>
            struct HAZELCAST_API hz_serializer<byte> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_BYTE;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<bool> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_BOOLEAN;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<char> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_CHAR;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<char16_t> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_CHAR;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<int16_t> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_SHORT;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<int32_t> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_INTEGER;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<int64_t> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_LONG;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<float> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_FLOAT;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<double> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_DOUBLE;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::string> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_STRING;
                }
            };


            template<>
            struct HAZELCAST_API hz_serializer<HazelcastJsonValue> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<byte>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_BYTE_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<bool>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<char>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_CHAR_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<int16_t>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_SHORT_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<int32_t>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_INTEGER_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<int64_t>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_LONG_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<float>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_FLOAT_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<double>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<std::string>> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_STRING_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<boost::uuids::uuid> : public builtin_serializer {
            public:
                static inline pimpl::serialization_constants get_type_id() {
                    return pimpl::serialization_constants::CONSTANT_TYPE_UUID;
                }
            };

            enum struct field_type {
                TYPE_PORTABLE = 0,
                TYPE_BYTE = 1,
                TYPE_BOOLEAN = 2,
                TYPE_CHAR = 3,
                TYPE_SHORT = 4,
                TYPE_INT = 5,
                TYPE_LONG = 6,
                TYPE_FLOAT = 7,
                TYPE_DOUBLE = 8,
                TYPE_UTF = 9,
                TYPE_PORTABLE_ARRAY = 10,
                TYPE_BYTE_ARRAY = 11,
                TYPE_BOOLEAN_ARRAY = 12,
                TYPE_CHAR_ARRAY = 13,
                TYPE_SHORT_ARRAY = 14,
                TYPE_INT_ARRAY = 15,
                TYPE_LONG_ARRAY = 16,
                TYPE_FLOAT_ARRAY = 17,
                TYPE_DOUBLE_ARRAY = 18,
                TYPE_UTF_ARRAY = 19
            };

            /**
            * ClassDefinition defines a class schema for portable classes. It allows to query field names, types, class id etc.
            * It can be created manually using {@link ClassDefinitionBuilder}
            * or on demand during serialization phase.
            *
            * @see ClassDefinitionBuilder
            */
            /**
            * FieldDefinition defines name, type, index of a field
            */
            class HAZELCAST_API FieldDefinition {
            public:

                /**
                * Constructor
                */
                FieldDefinition();

                /**
                * Constructor
                */
                FieldDefinition(int, const std::string &, field_type const &type, int version);

                /**
                * Constructor
                */
                FieldDefinition(int index, const std::string &field_name, field_type const &type, int factory_id,
                                int class_id, int version);

                /**
                * @return field type
                */
                const field_type &get_type() const;

                /**
                * @return field name
                */
                std::string get_name() const;

                /**
                * @return field index
                */
                int get_index() const;

                /**
                * @return factory id of this field's class
                */
                int get_factory_id() const;

                /**
                * @return class id of this field's class
                */
                int get_class_id() const;


                /**
                * @param dataOutput writes fieldDefinition to given dataOutput
                */
                void write_data(pimpl::DataOutput &data_output);

                /**
                * @param dataInput reads fieldDefinition from given dataOutput
                */
                void read_data(ObjectDataInput &data_input);

                bool operator==(const FieldDefinition &rhs) const;

                bool operator!=(const FieldDefinition &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition);

            private:
                int index_;
                std::string field_name_;
                field_type type_;
                int class_id_;
                int factory_id_;
                int version_;
            };

            class HAZELCAST_API ClassDefinition {
            public:

                /**
                * Constructor
                */
                ClassDefinition();

                /**
                * Constructor
                * @param factoryId factory id of class
                * @param classId id of class
                * @param version version of portable class
                */
                ClassDefinition(int factory_id, int class_id, int version);

                /**
                * Internal API
                * @param fieldDefinition to be added
                */
                void add_field_def(FieldDefinition &field_definition);

                /**
                * @param fieldName field name
                * @return true if this class definition contains a field named by given name
                */
                bool has_field(const std::string &field_name) const;

                /**
                * @param fieldName name of the field
                * @return field definition by given name
                * @throws IllegalArgumentException when field not found
                */
                const FieldDefinition &get_field(const std::string &field_name) const;

                /**
                * @param fieldName name of the field
                * @return type of given field
                * @throws IllegalArgumentException
                */
                field_type get_field_type(const std::string &field_name) const;

                /**
                * @return total field count
                */
                int get_field_count() const;

                /**
                * @return factory id
                */
                int get_factory_id() const;

                /**
                * @return class id
                */
                int get_class_id() const;

                /**
                * @return version
                */
                int get_version() const;

                /**
                * Internal API
                * @param newVersion portable version
                */
                void set_version_if_not_set(int new_version);

                /**
                * Internal API
                * @param dataOutput
                */
                void write_data(pimpl::DataOutput &data_output);

                /**
                * Internal API
                * @param dataInput
                */
                void read_data(ObjectDataInput &data_input);

                bool operator==(const ClassDefinition &rhs) const;

                bool operator!=(const ClassDefinition &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition);

            private:
                int factory_id_;
                int class_id_;
                int version_;

                ClassDefinition(const ClassDefinition &) = delete;

                ClassDefinition &operator=(const ClassDefinition &rhs) = delete;

                std::unordered_map<std::string, FieldDefinition> field_definitions_map_;

                std::unique_ptr<std::vector<byte> > binary_;

            };

            /**
            * ClassDefinitionBuilder is used to build and register ClassDefinitions manually.
            *
            * @see ClassDefinition
            * @see SerializationConfig#addClassDefinition(ClassDefinition)
            */
            class HAZELCAST_API ClassDefinitionBuilder {
            public:
                ClassDefinitionBuilder(int factory_id, int class_id, int version);

                ClassDefinitionBuilder &
                add_portable_field(const std::string &field_name, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder &
                add_portable_array_field(const std::string &field_name, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder &add_field(FieldDefinition &field_definition);

                void add_field(const std::string &field_name, field_type const &field_type);

                std::shared_ptr<ClassDefinition> build();

                int get_factory_id();

                int get_class_id();

                int get_version();
            private:
                int factory_id_;
                int class_id_;
                int version_;
                int index_;
                bool done_;

                std::vector<FieldDefinition> field_definitions_;

                void check();
            };

            class PortableVersionHelper {
            public:
                template<typename T>
                static inline typename std::enable_if<std::is_base_of<versioned_portable_serializer, hz_serializer<T>>::value, int>::type
                get_version(int) {
                    return hz_serializer<T>::get_class_version();
                }

                template<typename T>
                static inline typename std::enable_if<!std::is_base_of<versioned_portable_serializer, hz_serializer<T>>::value, int>::type
                get_version(int default_version) {
                    return default_version;
                }
            };

            class HAZELCAST_API ObjectDataInput : public pimpl::DataInput<std::vector<byte>> {
            public:
                /**
                * Internal API. Constructor
                */
                ObjectDataInput(const std::vector<byte> &buffer, int offset, pimpl::PortableSerializer &portable_ser,
                                pimpl::DataSerializer &data_ser,
                                std::shared_ptr<serialization::global_serializer> global_serializer);

                /**
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                typename std::enable_if<!(std::is_array<T>::value &&
                                          std::is_same<typename std::remove_all_extents<T>::type, char>::value), boost::optional<T>>::type
                inline read_object();

                template<typename T>
                typename std::enable_if<std::is_array<T>::value &&
                                          std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
                inline read_object();

                template<typename T>
                typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline read_object(int32_t type_id);

                template<typename T>
                typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline read_object(int32_t type_id);

                template<typename T>
                typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline read_object(int32_t type_id);

                template<typename T>
                typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline read_object(int32_t type_id);

                /**
                 * Global serialization
                 * @tparam T The type to be deserialized to
                 * @param typeId the type id of the serilizer
                 * @return the deserialized object
                 */
                template<typename T>
                typename std::enable_if<!(std::is_base_of<identified_data_serializer, hz_serializer<T>>::value ||
                        std::is_base_of<portable_serializer, hz_serializer<T>>::value ||
                        std::is_base_of<builtin_serializer, hz_serializer<T>>::value ||
                        std::is_base_of<custom_serializer, hz_serializer<T>>::value), boost::optional<T>>::type
                inline read_object(int32_t type_id);

            private:
                pimpl::PortableSerializer &portable_serializer_;
                pimpl::DataSerializer &data_serializer_;
                std::shared_ptr<serialization::global_serializer> global_serializer_;
            };

            class HAZELCAST_API ObjectDataOutput : public pimpl::DataOutput {
                friend pimpl::DefaultPortableWriter;
            public:
                /**
                * Internal API Constructor
                */
                explicit ObjectDataOutput(bool dont_write = false, pimpl::PortableSerializer *portable_ser = nullptr,
                                 std::shared_ptr<serialization::global_serializer> global_serializer = nullptr);

                template<typename T>
                void write_object(const T *object);

                /* enable_if needed here since 'boost::optional<char [5]>' can not be composed this template match */
                template<typename T>
                typename std::enable_if<!(std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value), void>::type
                write_object(const boost::optional<T> &object);

                template<typename T>
                typename std::enable_if<std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value, void>::type
                inline write_object(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, void>::type
                inline write_object(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, void>::type
                inline write_object(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, void>::type
                inline write_object(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, void>::type
                inline write_object(const T &object);

                template<typename T>
                typename std::enable_if<!(std::is_base_of<builtin_serializer, hz_serializer<T>>::value ||
                                          std::is_base_of<identified_data_serializer, hz_serializer<T>>::value ||
                                          std::is_base_of<portable_serializer, hz_serializer<T>>::value ||
                                          std::is_base_of<custom_serializer, hz_serializer<T>>::value ||
                                          (std::is_array<T>::value &&
                                           std::is_same<typename std::remove_all_extents<T>::type, char>::value)), void>::type
                inline write_object(const T object);

                void write_objects() {}

                template<typename FirstObjectType, typename ...OtherObjects>
                inline void write_objects(const FirstObjectType &object, const OtherObjects &...objects) {
                    write_object(object);
                    write_objects(objects...);
                }

                template<typename T>
                inline void write_bytes(const T &s) {
                    for (auto c : s) {
                        output_stream_.push_back(c);
                    }
                }

            private:
                pimpl::PortableSerializer *portable_serializer_;
                std::shared_ptr<serialization::global_serializer> global_serializer_;
            };

            template<>
            void HAZELCAST_API ObjectDataOutput::write_object(const char *object);

            namespace pimpl {
                class HAZELCAST_API PortableContext {
                public:
                    PortableContext(const SerializationConfig &serialization_conf);

                    int get_class_version(int factory_id, int class_id);

                    void set_class_version(int factory_id, int class_id, int version);

                    std::shared_ptr<ClassDefinition> lookup_class_definition(int factory_id, int class_id, int version);

                    std::shared_ptr<ClassDefinition> register_class_definition(std::shared_ptr<ClassDefinition>);

                    template<typename T>
                    std::shared_ptr<ClassDefinition> lookup_or_register_class_definition(const T &portable);

                    int get_version();

                    std::shared_ptr<ClassDefinition> read_class_definition(ObjectDataInput &input, int id, int class_id,
                                                                         int version);

                    const SerializationConfig &get_serialization_config() const;

                    template<typename T>
                    typename std::enable_if<std::is_same<byte, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_BYTE; }

                    template<typename T>
                    typename std::enable_if<std::is_same<char, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_CHAR; }


                    template<typename T>
                    typename std::enable_if<std::is_same<char16_t, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_CHAR; }

                    template<typename T>
                    typename std::enable_if<std::is_same<bool, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_BOOLEAN; }

                    template<typename T>
                    typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_SHORT; }

                    template<typename T>
                    typename std::enable_if<std::is_same<int32_t, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_INT; }

                    template<typename T>
                    typename std::enable_if<std::is_same<int64_t, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_LONG; }

                    template<typename T>
                    typename std::enable_if<std::is_same<float, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_FLOAT; }

                    template<typename T>
                    typename std::enable_if<std::is_same<double, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_DOUBLE; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::string, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_UTF; }


                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<byte>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_BYTE_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<char>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_CHAR_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_BOOLEAN_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<int16_t>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_SHORT_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<int32_t>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_INT_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<int64_t>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_LONG_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<float>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_FLOAT_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<double>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_DOUBLE_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<std::string>, typename std::remove_cv<T>::type>::value, field_type>::type
                    static get_type() { return field_type::TYPE_UTF_ARRAY; }

                private:
                    PortableContext(const PortableContext &) = delete;

                    ClassDefinitionContext &get_class_definition_context(int factory_id);

                    void operator=(const PortableContext &) = delete;

                    util::SynchronizedMap<int, ClassDefinitionContext> class_def_context_map_;
                    const SerializationConfig &serialization_config_;
                };

                class ClassDefinitionContext {
                public:

                    ClassDefinitionContext(int portable_context, PortableContext *p_context);

                    int get_class_version(int class_id);

                    void set_class_version(int class_id, int version);

                    std::shared_ptr<ClassDefinition> lookup(int, int);

                    std::shared_ptr<ClassDefinition> register_class_definition(std::shared_ptr<ClassDefinition>);

                private:
                    int64_t combine_to_long(int x, int y) const;

                    const int factory_id_;
                    util::SynchronizedMap<long long, ClassDefinition> versioned_definitions_;
                    util::SynchronizedMap<int, int> current_class_versions_;
                    PortableContext *portable_context_;
                };

                class HAZELCAST_API ClassDefinitionWriter {
                public:
                    ClassDefinitionWriter(PortableContext &portable_context, ClassDefinitionBuilder &builder);

                    template <typename T>
                    void write(const std::string &field_name, T value) {
                        typedef typename std::remove_pointer<typename std::remove_reference<typename std::remove_cv<T>::type>::type>::type value_type;
                        builder_.add_field(field_name, PortableContext::get_type<value_type>());
                    }

                    template<typename T>
                    void write_null_portable(const std::string &field_name) {
                        T portable;
                        int32_t factoryId = hz_serializer<T>::getFactoryId();
                        int32_t classId = hz_serializer<T>::getClassId();
                        std::shared_ptr<ClassDefinition> nestedClassDef = context_.lookup_class_definition(factoryId,
                                                                                                        classId,
                                                                                                        context_.get_version());
                        if (!nestedClassDef) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("ClassDefWriter::write_null_portable",
                                                                               "Cannot write null portable without explicitly registering class definition!"));
                        }
                        builder_.add_portable_field(field_name, nestedClassDef);
                    }

                    template<typename T>
                    void write_portable(const std::string &field_name, const T *portable) {
                        if (NULL == portable) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("ClassDefinitionWriter::write_portable",
                                                                               "Cannot write null portable without explicitly registering class definition!"));
                        }

                        std::shared_ptr<ClassDefinition> nestedClassDef = create_nested_class_def(*portable);
                        builder_.add_portable_field(field_name, nestedClassDef);
                    };

                    template<typename T>
                    void write_portable_array(const std::string &field_name, const std::vector<T> *portables) {
                        if (NULL == portables || portables->size() == 0) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ClassDefinitionWriter::write_portableArray",
                                                                  "Cannot write null portable array without explicitly registering class definition!"));
                        }
                        std::shared_ptr<ClassDefinition> nestedClassDef = create_nested_class_def((*portables)[0]);
                        builder_.add_portable_array_field(field_name, nestedClassDef);
                    };

                    std::shared_ptr<ClassDefinition> register_and_get();

                    ObjectDataOutput &get_raw_data_output();

                    void end();

                private:
                    template<typename T>
                    std::shared_ptr<ClassDefinition> create_nested_class_def(const T &portable);

                    ClassDefinitionBuilder &builder_;
                    PortableContext &context_;
                    ObjectDataOutput empty_data_output_;
                };

                class HAZELCAST_API PortableReaderBase {
                public:
                    PortableReaderBase(PortableSerializer &portable_ser,
                                       ObjectDataInput &input,
                                       std::shared_ptr<ClassDefinition> cd);

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
                                            std::is_same<std::string, typename std::remove_cv<T>::type>::value, T>::type
                        read(const std::string &field_name) {
                        set_position(field_name, PortableContext::get_type<T>());
                        return data_input_->read<T>();
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<boost::optional<std::string>, typename std::remove_cv<T>::type>::value, T>::type
                        read(const std::string &field_name) {
                        set_position(field_name, PortableContext::get_type<T>());
                        return data_input_->read<T>();
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<std::vector<byte>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<char>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<int16_t>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<int32_t>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<int64_t>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<float>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<double>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<std::string>, typename std::remove_cv<T>::type>::value, boost::optional<T>>::type
                                    read(const std::string &field_name) {
                        set_position(field_name, PortableContext::get_type<T>());
                        return data_input_->read<T>();
                    }

                    ObjectDataInput &get_raw_data_input();

                    void end();

                protected:
                    void set_position(const std::string &field_name, field_type const &field_type);

                    void check_factory_and_class(FieldDefinition fd, int factory_id, int class_id) const;

                    template<typename T>
                    boost::optional<T> get_portable_instance(const std::string &field_name);

                    std::shared_ptr<ClassDefinition> cd_;
                    ObjectDataInput *data_input_;
                    PortableSerializer *portable_serializer_;
                private:
                    int final_position_;
                    int offset_;
                    bool raw_;

                };

                class HAZELCAST_API DefaultPortableReader : public PortableReaderBase {
                public:
                    DefaultPortableReader(PortableSerializer &portable_ser,
                                          ObjectDataInput &input, std::shared_ptr<ClassDefinition> cd);

                    template<typename T>
                    boost::optional<T> read_portable(const std::string &field_name);

                    template<typename T>
                    boost::optional<std::vector<T>> read_portable_array(const std::string &field_name);
                };

                class HAZELCAST_API MorphingPortableReader : public PortableReaderBase {
                public:
                    MorphingPortableReader(PortableSerializer &portable_ser, ObjectDataInput &input,
                                           std::shared_ptr<ClassDefinition> cd);

                    template <typename T>
                    typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<float, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<double, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &field_name) {
                        if (!cd_->has_field(field_name)) {
                            return 0;
                        }
                        const field_type &currentFieldType = cd_->get_field_type(field_name);
                        return read_morphing<T>(currentFieldType, field_name);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<byte, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<char, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<char16_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<bool, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &field_name) {
                        if (!cd_->has_field(field_name)) {
                            return 0;
                        }
                        return PortableReaderBase::read<T>(field_name);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<std::string, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &field_name) {
                        if (!cd_->has_field(field_name)) {
                            return std::string();
                        }
                        return PortableReaderBase::read<T>(field_name);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<boost::optional<std::string>, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &field_name) {
                        if (!cd_->has_field(field_name)) {
                            return boost::none;
                        }
                        return PortableReaderBase::read<T>(field_name);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<std::vector<byte>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<char>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<int16_t>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<int32_t>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<int64_t>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<float>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<double>, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<std::vector<std::string>, typename std::remove_cv<T>::type>::value, boost::optional<T>>::type
                    read(const std::string &field_name) {
                        if (!cd_->has_field(field_name)) {
                            return boost::none;
                        }
                        return PortableReaderBase::read<T>(field_name);
                    }

                    template<typename T>
                    boost::optional<T> read_portable(const std::string &field_name);

                    template<typename T>
                    boost::optional<std::vector<T>> read_portable_array(const std::string &field_name);

                private:
                    template <typename T>
                    typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value, T>::type
                    read_morphing(field_type current_field_type, const std::string &field_name) {
                        switch(current_field_type) {
                            case field_type::TYPE_BYTE:
                                return PortableReaderBase::read<byte>(field_name);
                            case field_type::TYPE_SHORT:
                                return PortableReaderBase::read<int16_t>(field_name);
                            default:
                                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                                 "IncompatibleClassChangeError"));
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<int32_t, typename std::remove_cv<T>::type>::value, T>::type
                    read_morphing(field_type current_field_type, const std::string &field_name) {
                        switch(current_field_type) {
                            case field_type::TYPE_INT:
                                return PortableReaderBase::read<int32_t>(field_name);
                            case field_type::TYPE_CHAR:
                                return PortableReaderBase::read<char>(field_name);
                            default:
                                return read_morphing<int16_t>(current_field_type, field_name);
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<int64_t, typename std::remove_cv<T>::type>::value, T>::type
                    read_morphing(field_type current_field_type, const std::string &field_name) {
                        switch(current_field_type) {
                            case field_type::TYPE_LONG:
                                return PortableReaderBase::read<int64_t>(field_name);
                            default:
                                return read_morphing<int32_t>(current_field_type, field_name);
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<float, typename std::remove_cv<T>::type>::value, T>::type
                    read_morphing(field_type current_field_type, const std::string &field_name) {
                        switch(current_field_type) {
                            case field_type::TYPE_FLOAT:
                                return PortableReaderBase::read<float>(field_name);
                            default:
                                return static_cast<float>(read_morphing<int32_t>(current_field_type, field_name));
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<double, typename std::remove_cv<T>::type>::value, T>::type
                    read_morphing(field_type current_field_type, const std::string &field_name) {
                        switch(current_field_type) {
                            case field_type::TYPE_DOUBLE:
                                return PortableReaderBase::read<double>(field_name);
                            case field_type::TYPE_FLOAT:
                                return PortableReaderBase::read<float>(field_name);
                            default:
                                return static_cast<double>(read_morphing<int64_t>(current_field_type, field_name));
                        }
                    }
                };

                class DefaultPortableWriter;
                class HAZELCAST_API PortableSerializer {
                    friend DefaultPortableWriter;
                public:
                    PortableSerializer(PortableContext &portable_context);

                    template<typename T>
                    T read_object(ObjectDataInput &in);

                    template<typename T>
                    T read(ObjectDataInput &in, int32_t factory_id, int32_t class_id);

                    template<typename T>
                    void write(const T &object, ObjectDataOutput &out);

                private:
                    PortableContext &context_;

                    template<typename T>
                    int find_portable_version(int factory_id, int class_id) const;

                    PortableReader create_reader(ObjectDataInput &input, int factory_id, int class_id, int version,
                                                int portable_version);

                    int32_t read_int(ObjectDataInput &in) const;

                    template<typename T>
                    void write_internal(const T &object, ObjectDataOutput &out);

                    template<typename T>
                    void write_internal(const T &object, std::shared_ptr<ClassDefinition> &cd, ObjectDataOutput &out);

                    template<typename T>
                    std::shared_ptr<ClassDefinition> lookup_or_register_class_definition(const T &portable);
                };

                class HAZELCAST_API DataSerializer {
                public:
                    template<typename T>
                    static boost::optional<T> read_object(ObjectDataInput &in) {
                        bool identified = in.read<bool>();
                        if (!identified) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ObjectDataInput::read_object<identified_data_serializer>",
                                                                  "Received data is not identified data serialized."));
                        }

                        int32_t expectedFactoryId = hz_serializer<T>::get_factory_id();
                        int32_t expectedClassId = hz_serializer<T>::get_class_id();
                        int32_t factoryId = in.read<int32_t>();
                        int32_t classId = in.read<int32_t>();
                        if (expectedFactoryId != factoryId || expectedClassId != classId) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ObjectDataInput::read_object<identified_data_serializer>",
                                                                  (boost::format(
                                                                          "Factory id %1% and class id %2% of data do not match expected "
                                                                          "factory id %3% and class id %4%!") %
                                                                   factoryId % classId % expectedFactoryId %
                                                                   expectedClassId).str())
                            );
                        }

                        return boost::make_optional(hz_serializer<T>::read_data(in));
                    }

                    template<typename T>
                    static void write(const T &object, ObjectDataOutput &out);

                private:
                    int32_t read_int(ObjectDataInput &in) const;
                };

                class HAZELCAST_API DefaultPortableWriter {
                public:
                    DefaultPortableWriter(PortableSerializer &portable_ser, std::shared_ptr<ClassDefinition> cd,
                                          ObjectDataOutput &output);

                    ObjectDataOutput &get_raw_data_output();

                    template <typename T>
                    void write(const std::string &field_name, T value) {
                        typedef typename std::remove_pointer<typename std::remove_reference<typename std::remove_cv<T>::type>::type>::type value_type;
                        set_position(field_name, PortableContext::get_type<value_type>());
                        object_data_output_.write(value);
                    }

                    void end();

                    template<typename T>
                    void write_null_portable(const std::string &field_name);

                    template<typename T>
                    void write_portable(const std::string &field_name, const T *portable);

                    template<typename T>
                    void write_portable_array(const std::string &field_name, const std::vector<T> *values);

                private:
                    FieldDefinition const &set_position(const std::string &field_name, field_type field_type);

                    template<typename T>
                    void check_portable_attributes(const FieldDefinition &fd);

                    bool raw_;
                    PortableSerializer &portable_serializer_;
                    ObjectDataOutput &object_data_output_;
                    size_t begin_;
                    size_t offset_;
                    std::unordered_set<std::string> written_fields_;
                    std::shared_ptr<ClassDefinition> cd_;
                };

                class HAZELCAST_API SerializationService : public util::Disposable {
                public:
                    SerializationService(const SerializationConfig &config);

                    PortableSerializer &get_portable_serializer();

                    DataSerializer &get_data_serializer();

                    template<typename T>
                    inline data to_data(const T *object) {
                        ObjectDataOutput output(false, &portable_serializer_, serialization_config_.get_global_serializer());

                        write_hash<T>(object, output);

                        output.write_object<T>(object);

                        return {std::move(output).to_byte_array()};
                    }

                    template<typename T>
                    inline data to_data(const T &object) {
                        ObjectDataOutput output(false, &portable_serializer_, serialization_config_.get_global_serializer());

                        write_hash<T>(&object, output);

                        output.write_object<T>(object);

                        return {std::move(output).to_byte_array()};
                    }

                    template<typename T>
                    inline std::shared_ptr<data> to_shared_data(const T *object) {
                        if (NULL == object) {
                            return std::shared_ptr<data>();
                        }
                        return std::shared_ptr<data>(new data(to_data<T>(object)));
                    }

                    template<typename T>
                    inline boost::optional<T> to_object(const data *data) {
                        if (!data) {
                            return boost::none;
                        }
                        return to_object<T>(*data);
                    }

                    template<typename T>
                    typename std::enable_if<!(std::is_same<T, const char *>::value ||
                                              std::is_same<T, const char *>::value ||
                                              std::is_same<T, typed_data>::value), boost::optional<T>>::type
                    inline to_object(const data &data) {
                        if (is_null_data(data)) {
                            return boost::none;
                        }

                        int32_t typeId = data.get_type();

                        // Constant 8 is Data::DATA_OFFSET. Windows DLL export does not
                        // let usage of static member.
                        ObjectDataInput objectDataInput(data.to_byte_array(), 8, portable_serializer_, data_serializer_,
                                                        serialization_config_.get_global_serializer());
                        return objectDataInput.read_object<T>(typeId);
                    }

                    template<typename T>
                    typename std::enable_if<std::is_same<T, typed_data>::value, boost::optional<T>>::type
                    inline to_object(const data &d) {
                        return boost::make_optional(typed_data(data(d), *this));
                    }

                    template<typename T>
                    typename std::enable_if<std::is_same<T, const char *>::value, boost::optional<std::string>>::type
                    inline to_object(const data &data) {
                        return to_object<std::string>(data);
                    }

                    template<typename T>
                    typename std::enable_if<std::is_array<T>::value &&
                                            std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
                    inline to_object(const data &data) {
                        return to_object<std::string>(data);
                    }

                    template<typename T>
                    inline std::shared_ptr<data> to_shared_object(const std::shared_ptr<data> &data) {
                        return data;
                    }

                    const byte get_version() const;

                    ObjectType get_object_type(const data *data);

                    /**
                     * @link Disposable interface implementation
                     */
                    void dispose() override;

                    ObjectDataOutput new_output_stream();
                private:
                    SerializationService(const SerializationService &) = delete;

                    SerializationService &operator=(const SerializationService &) = delete;

                    const SerializationConfig &serialization_config_;
                    PortableContext portable_context_;
                    serialization::pimpl::PortableSerializer portable_serializer_;
                    serialization::pimpl::DataSerializer data_serializer_;

                    static bool is_null_data(const data &data);

                    template<typename T>
                    void write_hash(const PartitionAwareMarker *obj, DataOutput &out) {
                        typedef typename T::KEY_TYPE PK_TYPE;
                        const PartitionAware<PK_TYPE> *partitionAwareObj = static_cast<const PartitionAware<PK_TYPE> *>(obj);
                        const PK_TYPE *pk = partitionAwareObj->get_partition_key();
                        if (pk != NULL) {
                            data partitionKey = to_data<PK_TYPE>(pk);
                            out.write<int32_t>(partitionKey.get_partition_hash());
                        }
                    }

                    template<typename T>
                    void write_hash(const void *obj, DataOutput &out) {
                        out.write<int32_t>(0);
                    }
                };

                template<>
                data HAZELCAST_API SerializationService::to_data(const char *object);
            }

            /**
            * Provides a mean of reading portable fields from a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableReader {
            public:
                PortableReader(pimpl::PortableSerializer &portable_ser, ObjectDataInput &data_input,
                               const std::shared_ptr<ClassDefinition>& cd, bool is_default_reader);

                /**
                * @param fieldName name of the field
                * @return the value read
                */
                template <typename T>
                typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<float, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<double, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<byte, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<char, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<char16_t, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<bool, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::string, typename std::remove_cv<T>::type>::value, T>::type
                read(const std::string &field_name) {
                    if (is_default_reader_)
                        return default_portable_reader_->read<T>(field_name);
                    return morphing_portable_reader_->read<T>(field_name);
                }

                /**
                * @param fieldName name of the field
                * @return the value read
                */
                template<typename T>
                typename std::enable_if<std::is_same<std::vector<byte>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<char>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<int16_t>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<int32_t>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<int64_t>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<float>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<double>, typename std::remove_cv<T>::type>::value ||
                                        std::is_same<std::vector<std::string>, typename std::remove_cv<T>::type>::value, boost::optional<T>>::type
                read(const std::string &field_name) {
                    if (is_default_reader_)
                        return default_portable_reader_->read<T>(field_name);
                    return morphing_portable_reader_->read<T>(field_name);
                }

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @return the portable value read
                */
                template<typename T>
                boost::optional<T> read_portable(const std::string &field_name);

                /**
                * @tparam type of the portable class in array
                * @param fieldName name of the field
                * @return the portable array value read
                */
                template<typename T>
                boost::optional<std::vector<T>> read_portable_array(const std::string &field_name);

                /**
                * @see PortableWriter#getRawDataOutput
                *
                * Note that portable fields can not read after getRawDataInput() is called. In case this happens,
                * IOException will be thrown.
                *
                * @return rawDataInput
                * @throws IOException
                */
                ObjectDataInput &get_raw_data_input();

                /**
                * Internal Api. Should not be called by end user.
                */
                void end();

            private:
                bool is_default_reader_;
                boost::optional<pimpl::DefaultPortableReader> default_portable_reader_;
                boost::optional<pimpl::MorphingPortableReader> morphing_portable_reader_;
            };

            /**
            * Provides a mean of writing portable fields to a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableWriter {
            public:
                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::DefaultPortableWriter *default_portable_writer);

                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::ClassDefinitionWriter *class_definition_writer);

                template <typename T>
                void write(const std::string &field_name, T value) {
                    if(is_default_writer_) {
                        default_portable_writer_->write(field_name, value);
                    } else {
                        class_definition_writer_->write(field_name, value);
                    }
                }

                /**
                * Internal api , should not be called by end user.
                */
                void end();

                /**
                * To write a null portable value.
                *
                * @tparam type of the portable field
                * @param fieldName name of the field
                *
                * @throws IOException
                */
                template<typename T>
                void write_null_portable(const std::string &field_name);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param portable  Portable to be written
                * @throws IOException
                */
                template<typename T>
                void write_portable(const std::string &field_name, const T *portable);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param values portable array to be written
                * @throws IOException
                */
                template<typename T>
                void write_portable_array(const std::string &field_name, const std::vector<T> *values);

                /**
                * After writing portable fields, one can write remaining fields in old fashioned way consecutively at the end
                * of stream. User should not that after getting rawDataOutput trying to write portable fields will result
                * in IOException
                *
                * @return ObjectDataOutput
                * @throws IOException
                */
                ObjectDataOutput &get_raw_data_output();

            private:
                pimpl::DefaultPortableWriter *default_portable_writer_;
                pimpl::ClassDefinitionWriter *class_definition_writer_;
                bool is_default_writer_;
            };

            template<typename T>
            boost::optional<T> PortableReader::read_portable(const std::string &field_name) {
                if (is_default_reader_)
                    return default_portable_reader_->read_portable<T>(field_name);
                return morphing_portable_reader_->read_portable<T>(field_name);
            }

            /**
            * @tparam type of the portable class in array
            * @param fieldName name of the field
            * @return the portable array value read
            * @throws IOException
            */
            template<typename T>
            boost::optional<std::vector<T>> PortableReader::read_portable_array(const std::string &field_name) {
                if (is_default_reader_)
                    return default_portable_reader_->read_portable_array<T>(field_name);
                return morphing_portable_reader_->read_portable_array<T>(field_name);
            };

            template<typename T>
            void PortableWriter::write_null_portable(const std::string &field_name) {
                if (is_default_writer_)
                    return default_portable_writer_->write_null_portable<T>(field_name);
                return class_definition_writer_->write_null_portable<T>(field_name);
            }

            /**
            * @tparam type of the portable class
            * @param fieldName name of the field
            * @param portable  Portable to be written
            * @throws IOException
            */
            template<typename T>
            void PortableWriter::write_portable(const std::string &field_name, const T *portable) {
                if (is_default_writer_)
                    return default_portable_writer_->write_portable(field_name, portable);
                return class_definition_writer_->write_portable(field_name, portable);

            }

            /**
            * @tparam type of the portable class
            * @param fieldName name of the field
            * @param values portable array to be written
            * @throws IOException
            */
            template<typename T>
            void PortableWriter::write_portable_array(const std::string &field_name, const std::vector<T> *values) {
                if (is_default_writer_)
                    return default_portable_writer_->write_portable_array(field_name, values);
                return class_definition_writer_->write_portable_array(field_name, values);
            }

            template<typename T>
            void ObjectDataOutput::write_object(const T *object) {
                if (is_no_write_) { return; }
                if (!object) {
                    write<int32_t>(static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_NULL));
                    return;
                }

                write_object<T>(*object);
            }

            template<typename T>
            typename std::enable_if<!(std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value), void>::type
            ObjectDataOutput::write_object(const boost::optional<T> &object) {
                if (is_no_write_) { return; }
                if (!object) {
                    write<int32_t>(static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_NULL));
                    return;
                }

                write_object<T>(object.value());
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::write_object(const T &object) {
                if (is_no_write_) { return; }
                write<int32_t>(static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_DATA));
                pimpl::DataSerializer::write<T>(object, *this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::write_object(const T &object) {
                if (is_no_write_) { return; }
                write<int32_t>(static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_PORTABLE));
                portable_serializer_->write<T>(object, *this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::write_object(const T &object) {
                if (is_no_write_) { return; }
                write<int32_t>(static_cast<int32_t>((hz_serializer<T>::get_type_id())));
                write < T > (object);
            }

            template<typename T>
            typename std::enable_if<std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value, void>::type
            inline ObjectDataOutput::write_object(const T &object) {
                write_object(std::string(object));
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::write_object(const T &object) {
                if (is_no_write_) { return; }
                static_assert(hz_serializer<T>::get_type_id() > 0, "Custom serializer type id can not be negative!");
                write<int32_t>(hz_serializer<T>::get_type_id());
                hz_serializer<T>::write(object, *this);
            }

            /**
             * Global serialization if configured
             * @tparam T
             * @param object
             * @return
             */
            template<typename T>
            typename std::enable_if<!(std::is_base_of<builtin_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<identified_data_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<portable_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<custom_serializer, hz_serializer<T>>::value ||
                                      (std::is_array<T>::value &&
                                       std::is_same<typename std::remove_all_extents<T>::type, char>::value)), void>::type
            inline ObjectDataOutput::write_object(const T object) {
                if (!global_serializer_) {
                    throw exception::HazelcastSerializationException("ObjectDataOutput::write_object",
                            (boost::format("No serializer found for type(%1%).") %typeid(T).name()).str());
                }
                if (is_no_write_) { return; }
                write<int32_t>(static_cast<int32_t>(global_serializer::get_type_id()));
                global_serializer_->write(boost::any(std::move(object)), *this);
            }

            template<typename T>
            typename std::enable_if<!(std::is_array<T>::value &&
                                      std::is_same<typename std::remove_all_extents<T>::type, char>::value), boost::optional<T>>::type
            inline ObjectDataInput::read_object() {
                int32_t typeId = read<int32_t>();
                if (static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_NULL) == typeId) {
                    return boost::none;
                }
                return read_object<T>(typeId);
            }

            template<typename T>
            typename std::enable_if<std::is_array<T>::value &&
                                    std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
            inline ObjectDataInput::read_object() {
                return read_object<std::string>();
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::read_object(int32_t type_id) {
                if (type_id != static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_DATA)) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                  "ObjectDataInput::read_object<identified_data_serializer>",
                                                          (boost::format(
                                                                  "The associated serializer Serializer<T> is identified_data_serializer "
                                                                  "but received data type id is %1%") % type_id).str()));
                }

                return data_serializer_.read_object<T>(*this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::read_object(int32_t type_id) {
                if (type_id != static_cast<int32_t>(pimpl::serialization_constants::CONSTANT_TYPE_PORTABLE)) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                  "ObjectDataInput::read_object<portable_serializer>",
                                                          (boost::format(
                                                                  "The associated serializer Serializer<T> is portable_serializer "
                                                                  "but received data type id is %1%") % type_id).str()));
                }

                return portable_serializer_.read_object<T>(*this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::read_object(int32_t type_id) {
                if (type_id != hz_serializer<T>::get_type_id()) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ObjectDataInput::read_object<>",
                                                                                     (boost::format(
                                                                                             "The associated serializer Serializer<T> type id %1% does not match "
                                                                                             "received data type id is %2%") %
                                                                                      hz_serializer<T>::get_type_id() %
                                                                                      type_id).str()));
                }

                return boost::optional<T>(hz_serializer<T>::read(*this));
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::read_object(int32_t type_id) {
                assert(type_id == static_cast<int32_t>(hz_serializer<T>::get_type_id()));

                return boost::optional<T>(read<T>());
            }

            template<typename T>
            typename std::enable_if<!(std::is_base_of<identified_data_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<portable_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<builtin_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<custom_serializer, hz_serializer<T>>::value), boost::optional<T>>::type
            inline ObjectDataInput::read_object(int32_t type_id) {
                if (!global_serializer_) {
                    throw exception::HazelcastSerializationException("ObjectDataInput::read_object",
                            (boost::format("No serializer found for type %1%.") %typeid(T).name()).str());
                }

                if (type_id != static_cast<int32_t>(global_serializer_->get_type_id())) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ObjectDataInput::read_object<>",
                                                                                     (boost::format(
                                                                                             "The global serializer type id %1% does not match "
                                                                                             "received data type id is %2%") %
                                                                                             static_cast<int32_t>(global_serializer_->get_type_id()) %
                                                                                      type_id).str()));
                }

                return boost::optional<T>(boost::any_cast<T>(std::move(global_serializer_->read(*this))));
            }

            namespace pimpl {
                template<>
                data SerializationService::to_data(const typed_data *object);

                template<typename T>
                boost::optional<T> DefaultPortableReader::read_portable(const std::string &field_name) {
                    return get_portable_instance<T>(field_name);
                }

                template<typename T>
                boost::optional<std::vector<T>> DefaultPortableReader::read_portable_array(const std::string &field_name) {
                    PortableReaderBase::set_position(field_name, field_type::TYPE_PORTABLE_ARRAY);

                    data_input_->read<int32_t>();
                    std::vector<T> portables;

                    set_position(field_name, field_type::TYPE_PORTABLE_ARRAY);

                    int32_t len = data_input_->read<int32_t>();
                    if (len == util::Bits::NULL_ARRAY) {
                        return boost::none;
                    }
                    int32_t factoryId = data_input_->read<int32_t>();
                    int32_t classId = data_input_->read<int32_t>();

                    check_factory_and_class(cd_->get_field(field_name), factoryId, classId);

                    if (len > 0) {
                        int offset = data_input_->position();
                        for (int i = 0; i < len; i++) {
                            data_input_->position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int32_t start = data_input_->read<int32_t>();
                            data_input_->position(start);

                            portables.push_back(portable_serializer_->read<T>(*data_input_, factoryId, classId));
                        }
                    }
                    return portables;
                }

                template<typename T>
                boost::optional<T> MorphingPortableReader::read_portable(const std::string &field_name) {
                    return get_portable_instance<T>(field_name);
                }

                template<typename T>
                boost::optional<std::vector<T>> MorphingPortableReader::read_portable_array(const std::string &field_name) {
                    PortableReaderBase::set_position(field_name, field_type::TYPE_PORTABLE_ARRAY);

                    data_input_->read<int32_t>();
                    std::vector<T> portables;

                    set_position(field_name, field_type::TYPE_PORTABLE_ARRAY);

                    int32_t len = data_input_->read<int32_t>();
                    if (len == util::Bits::NULL_ARRAY) {
                        return boost::none;
                    }
                    int32_t factoryId = data_input_->read<int32_t>();
                    int32_t classId = data_input_->read<int32_t>();

                    check_factory_and_class(cd_->get_field(field_name), factoryId, classId);

                    if (len > 0) {
                        portables.reserve(static_cast<size_t>(len));
                        int offset = data_input_->position();
                        for (int i = 0; i < len; i++) {
                            data_input_->position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int32_t start = data_input_->read<int32_t>();
                            data_input_->position(start);

                            portables.emplace_back(portable_serializer_->read<T>(*data_input_, factoryId, classId));
                        }
                    }

                    return boost::make_optional(std::move(portables));
                }

                template<typename T>
                T PortableSerializer::read_object(ObjectDataInput &in) {
                    int32_t factoryId = read_int(in);
                    int32_t classId = read_int(in);

                    if (factoryId != hz_serializer<T>::get_factory_id() || classId != hz_serializer<T>::get_class_id()) {
                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastSerializationException("PortableSerializer::read_object",
                                                                           (boost::format("Received data (factory-class id)=(%1%, %2%) does not match expected factory-class id (%3%, %4%)") %
                                                                            factoryId % classId %
                                                                            hz_serializer<T>::get_factory_id() %
                                                                            hz_serializer<T>::get_class_id()).str()));
                    }
                    return read<T>(in, factoryId, classId);
                }

                template<typename T>
                T PortableSerializer::read(ObjectDataInput &in, int32_t factory_id, int32_t class_id) {
                    int version = in.read<int32_t>();

                    int portableVersion = find_portable_version<T>(factory_id, class_id);

                    PortableReader reader = create_reader(in, factory_id, class_id, version, portableVersion);
                    T result = hz_serializer<T>::read_portable(reader);
                    reader.end();
                    return result;
                }

                template<typename T>
                void PortableSerializer::write(const T &object, ObjectDataOutput &out) {
                    out.write<int32_t>(hz_serializer<T>::get_factory_id());
                    out.write<int32_t>(hz_serializer<T>::get_class_id());

                    write_internal(object, out);
                }

                template<typename T>
                void PortableSerializer::write_internal(const T &object, ObjectDataOutput &out) {
                    auto cd = context_.lookup_or_register_class_definition<T>(object);
                    write_internal(object, cd, out);
                }

                template<typename T>
                void PortableSerializer::write_internal(const T &object, std::shared_ptr<ClassDefinition> &cd,
                                               ObjectDataOutput &out) {
                    out.write<int32_t>(cd->get_version());

                    DefaultPortableWriter dpw(*this, cd, out);
                    PortableWriter portableWriter(&dpw);
                    hz_serializer<T>::write_portable(object, portableWriter);
                    portableWriter.end();
                }

                template<typename T>
                std::shared_ptr<ClassDefinition> PortableSerializer::lookup_or_register_class_definition(const T &portable) {
                    return context_.lookup_or_register_class_definition<T>(portable);
                }

                template<typename T>
                int PortableSerializer::find_portable_version(int factory_id, int class_id) const {
                    int currentVersion = context_.get_class_version(factory_id, class_id);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::get_version<T>(context_.get_version());
                        if (currentVersion > 0) {
                            context_.set_class_version(factory_id, class_id, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                template<typename T>
                void DataSerializer::write(const T &object, ObjectDataOutput &out) {
                    out.write<bool>(true);
                    out.write<int32_t>(hz_serializer<T>::get_factory_id());
                    out.write<int32_t>(hz_serializer<T>::get_class_id());
                    hz_serializer<T>::write_data(object, out);
                }

                template<typename T>
                std::shared_ptr<ClassDefinition>
                PortableContext::lookup_or_register_class_definition(const T &portable) {
                    int portableVersion = PortableVersionHelper::get_version<T>(
                            serialization_config_.get_portable_version());
                    std::shared_ptr<ClassDefinition> cd = lookup_class_definition(hz_serializer<T>::get_factory_id(),
                                                                                hz_serializer<T>::get_class_id(),
                                                                                portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(hz_serializer<T>::get_factory_id(),
                                                                      hz_serializer<T>::get_class_id(), portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        hz_serializer<T>::write_portable(portable, portableWriter);
                        cd = cdw.register_and_get();
                    }
                    return cd;
                }

                template<typename T>
                boost::optional<T> PortableReaderBase::get_portable_instance(const std::string &field_name) {
                    set_position(field_name, field_type::TYPE_PORTABLE);

                    bool isNull = data_input_->read<bool>();
                    int32_t factoryId = data_input_->read<int32_t>();
                    int32_t classId = data_input_->read<int32_t>();

                    check_factory_and_class(cd_->get_field(field_name), factoryId, classId);

                    if (isNull) {
                        return boost::none;
                    } else {
                        return portable_serializer_->read<T>(*data_input_, factoryId, classId);
                    }
                }

                template<typename T>
                void DefaultPortableWriter::write_null_portable(const std::string &field_name) {
                    set_position(field_name, field_type::TYPE_PORTABLE);
                    object_data_output_.write<bool>(true);
                    object_data_output_.write<int32_t>(hz_serializer<T>::getFactoryId());
                    object_data_output_.write<int32_t>(hz_serializer<T>::getClassId());
                }

                template<typename T>
                void DefaultPortableWriter::write_portable(const std::string &field_name, const T *portable) {
                    FieldDefinition const &fd = set_position(field_name, field_type::TYPE_PORTABLE);
                    bool isNull = (nullptr == portable);
                    object_data_output_.write<bool>(isNull);

                    object_data_output_.write<int32_t>(hz_serializer<T>::get_factory_id());
                    object_data_output_.write<int32_t>(hz_serializer<T>::get_class_id());

                    if (!isNull) {
                        check_portable_attributes<T>(fd);
                        portable_serializer_.write_internal(*portable, object_data_output_);
                    }

                    portable_serializer_.write(*portable, object_data_output_);
                }

                template<typename T>
                void DefaultPortableWriter::write_portable_array(const std::string &field_name, const std::vector<T> *values) {
                    FieldDefinition const &fd = set_position(field_name, field_type::TYPE_PORTABLE_ARRAY);
                    check_portable_attributes<T>(fd);

                    int32_t len = (values ? static_cast<int32_t>(values->size()) : util::Bits::NULL_ARRAY);
                    object_data_output_.write<int32_t>(len);

                    object_data_output_.write<int32_t>(fd.get_factory_id());
                    object_data_output_.write<int32_t>(fd.get_class_id());

                    if (len > 0) {
                        std::shared_ptr<ClassDefinition> classDefinition = portable_serializer_.lookup_or_register_class_definition<T>(
                                (*values)[0]);
                        size_t currentOffset = object_data_output_.position();
                        object_data_output_.position(currentOffset + len * util::Bits::INT_SIZE_IN_BYTES);
                        for (int32_t i = 0; i < len; i++) {
                            size_t position = object_data_output_.position();
                            object_data_output_.write_at(currentOffset + i * util::Bits::INT_SIZE_IN_BYTES, static_cast<int32_t>(position));
                            portable_serializer_.write_internal((*values)[i], classDefinition, object_data_output_);
                        }
                    }
                }

                template<typename T>
                void DefaultPortableWriter::check_portable_attributes(const FieldDefinition &fd) {
                    if (fd.get_factory_id() != hz_serializer<T>::get_factory_id()) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", (boost::format(
                                                              "Wrong Portable type! Expected factory-id: %1%, Actual factory-id: %2%")
                                                              %fd.get_factory_id() %hz_serializer<T>::get_factory_id()).str()));
                    }
                    if (fd.get_class_id() != hz_serializer<T>::get_class_id()) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", (boost::format(
                                                              "Wrong Portable type! Expected class-id: %1%, Actual class-id: %2%")
                                                              %fd.get_class_id() %hz_serializer<T>::get_class_id()).str()));
                    }
                }

                template<typename T>
                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::create_nested_class_def(const T &portable) {
                    int version = PortableVersionHelper::get_version<T>(context_.get_version());
                    ClassDefinitionBuilder definitionBuilder(hz_serializer<T>::get_factory_id(), hz_serializer<T>::get_class_id(),
                                                             version);

                    ClassDefinitionWriter nestedWriter(context_, definitionBuilder);
                    PortableWriter portableWriter(&nestedWriter);
                    hz_serializer<T>::write_portable(portable, portableWriter);
                    return context_.register_class_definition(definitionBuilder.build());
                }
            }
        }

        template <typename T>
        boost::optional<T> typed_data::get() const {
            return ss_->to_object<T>(data_);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

