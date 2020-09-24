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
#include "hazelcast/client/serialization/pimpl/Data.h"
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

                enum struct HAZELCAST_API SerializationConstants {
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

                    SerializationConstants typeId;
                    int32_t factoryId;
                    int32_t classId;

                };

                std::ostream HAZELCAST_API &operator<<(std::ostream &os, const ObjectType &type);
            }
        }

        /**
         * TypedData class is a wrapper class for the serialized binary data. It does late deserialization of the data
         * only when the get method is called.
         */
        class HAZELCAST_API TypedData {
        public:
            TypedData();

            TypedData(serialization::pimpl::Data d,
                      serialization::pimpl::SerializationService &serializationService);

            /**
             *
             * @return The type of the underlying object for this binary.
             */
            serialization::pimpl::ObjectType getType() const;

            /**
             * Deserializes the underlying binary data and produces the object of type T.
             *
             * <b>CAUTION</b>: The type that you provide should be compatible with what object type is returned with
             * the getType API, otherwise you will either get an exception of incorrectly try deserialize the binary data.
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
            const serialization::pimpl::Data &getData() const;

        private:
            serialization::pimpl::Data data_;
            serialization::pimpl::SerializationService *ss_;
        };

        bool HAZELCAST_API operator<(const TypedData &lhs, const TypedData &rhs);

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

                static pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_GLOBAL;
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
             *      static int32_t writePortable(const T &object, PortableWriter &out);
             *      static T readPortable(PortableReader &in);
             */
            struct portable_serializer {
            };

            struct versioned_portable_serializer : public portable_serializer {
            };

            template<>
            struct HAZELCAST_API hz_serializer<byte> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<bool> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<char> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<char16_t> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<int16_t> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<int32_t> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<int64_t> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_LONG;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<float> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<double> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::string> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_STRING;
                }
            };


            template<>
            struct HAZELCAST_API hz_serializer<HazelcastJsonValue> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<byte>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<bool>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<char>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<int16_t>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<int32_t>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<int64_t>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<float>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<double>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<std::vector<std::string>> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }
            };

            template<>
            struct HAZELCAST_API hz_serializer<boost::uuids::uuid> : public builtin_serializer {
            public:
                static inline pimpl::SerializationConstants getTypeId() {
                    return pimpl::SerializationConstants::CONSTANT_TYPE_UUID;
                }
            };

            enum struct FieldType {
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
                FieldDefinition(int, const std::string &, FieldType const &type, int version);

                /**
                * Constructor
                */
                FieldDefinition(int index, const std::string &fieldName, FieldType const &type, int factoryId,
                                int classId, int version);

                /**
                * @return field type
                */
                const FieldType &getType() const;

                /**
                * @return field name
                */
                std::string getName() const;

                /**
                * @return field index
                */
                int getIndex() const;

                /**
                * @return factory id of this field's class
                */
                int getFactoryId() const;

                /**
                * @return class id of this field's class
                */
                int getClassId() const;


                /**
                * @param dataOutput writes fieldDefinition to given dataOutput
                */
                void writeData(pimpl::DataOutput &dataOutput);

                /**
                * @param dataInput reads fieldDefinition from given dataOutput
                */
                void readData(ObjectDataInput &dataInput);

                bool operator==(const FieldDefinition &rhs) const;

                bool operator!=(const FieldDefinition &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition);

            private:
                int index_;
                std::string field_name_;
                FieldType type_;
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
                ClassDefinition(int factoryId, int classId, int version);

                /**
                * Internal API
                * @param fieldDefinition to be added
                */
                void addFieldDef(FieldDefinition &fieldDefinition);

                /**
                * @param fieldName field name
                * @return true if this class definition contains a field named by given name
                */
                bool hasField(const std::string &fieldName) const;

                /**
                * @param fieldName name of the field
                * @return field definition by given name
                * @throws IllegalArgumentException when field not found
                */
                const FieldDefinition &getField(const std::string &fieldName) const;

                /**
                * @param fieldName name of the field
                * @return type of given field
                * @throws IllegalArgumentException
                */
                FieldType getFieldType(const std::string &fieldName) const;

                /**
                * @return total field count
                */
                int getFieldCount() const;

                /**
                * @return factory id
                */
                int getFactoryId() const;

                /**
                * @return class id
                */
                int getClassId() const;

                /**
                * @return version
                */
                int getVersion() const;

                /**
                * Internal API
                * @param newVersion portable version
                */
                void setVersionIfNotSet(int newVersion);

                /**
                * Internal API
                * @param dataOutput
                */
                void writeData(pimpl::DataOutput &dataOutput);

                /**
                * Internal API
                * @param dataInput
                */
                void readData(ObjectDataInput &dataInput);

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
                ClassDefinitionBuilder(int factoryId, int classId, int version);

                ClassDefinitionBuilder &
                addPortableField(const std::string &fieldName, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder &
                addPortableArrayField(const std::string &fieldName, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder &addField(FieldDefinition &fieldDefinition);

                void addField(const std::string &fieldName, FieldType const &fieldType);

                std::shared_ptr<ClassDefinition> build();

                int getFactoryId();

                int getClassId();

                int getVersion();
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
                getVersion(int) {
                    return hz_serializer<T>::getClassVersion();
                }

                template<typename T>
                static inline typename std::enable_if<!std::is_base_of<versioned_portable_serializer, hz_serializer<T>>::value, int>::type
                getVersion(int defaultVersion) {
                    return defaultVersion;
                }
            };

            class HAZELCAST_API ObjectDataInput : public pimpl::DataInput<std::vector<byte>> {
            public:
                /**
                * Internal API. Constructor
                */
                ObjectDataInput(const std::vector<byte> &buffer, int offset, pimpl::PortableSerializer &portableSer,
                                pimpl::DataSerializer &dataSer,
                                std::shared_ptr<serialization::global_serializer> globalSerializer);

                /**
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                typename std::enable_if<!(std::is_array<T>::value &&
                                          std::is_same<typename std::remove_all_extents<T>::type, char>::value), boost::optional<T>>::type
                inline readObject();

                template<typename T>
                typename std::enable_if<std::is_array<T>::value &&
                                          std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
                inline readObject();

                template<typename T>
                typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline readObject(int32_t typeId);

                template<typename T>
                typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline readObject(int32_t typeId);

                template<typename T>
                typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline readObject(int32_t typeId);

                template<typename T>
                typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline readObject(int32_t typeId);

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
                inline readObject(int32_t typeId);

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
                explicit ObjectDataOutput(bool dontWrite = false, pimpl::PortableSerializer *portableSer = nullptr,
                                 std::shared_ptr<serialization::global_serializer> globalSerializer = nullptr);

                template<typename T>
                void writeObject(const T *object);

                /* enable_if needed here since 'boost::optional<char [5]>' can not be composed this template match */
                template<typename T>
                typename std::enable_if<!(std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value), void>::type
                writeObject(const boost::optional<T> &object);

                template<typename T>
                typename std::enable_if<std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                typename std::enable_if<!(std::is_base_of<builtin_serializer, hz_serializer<T>>::value ||
                                          std::is_base_of<identified_data_serializer, hz_serializer<T>>::value ||
                                          std::is_base_of<portable_serializer, hz_serializer<T>>::value ||
                                          std::is_base_of<custom_serializer, hz_serializer<T>>::value ||
                                          (std::is_array<T>::value &&
                                           std::is_same<typename std::remove_all_extents<T>::type, char>::value)), void>::type
                inline writeObject(const T object);

                void writeObjects() {}

                template<typename FirstObjectType, typename ...OtherObjects>
                inline void writeObjects(const FirstObjectType &object, const OtherObjects &...objects) {
                    writeObject(object);
                    writeObjects(objects...);
                }

                template<typename T>
                inline void writeBytes(const T &s) {
                    for (auto c : s) {
                        outputStream.push_back(c);
                    }
                }

            private:
                pimpl::PortableSerializer *portable_serializer_;
                std::shared_ptr<serialization::global_serializer> global_serializer_;
            };

            template<>
            void HAZELCAST_API ObjectDataOutput::writeObject(const char *object);

            namespace pimpl {
                class HAZELCAST_API PortableContext {
                public:
                    PortableContext(const SerializationConfig &serializationConf);

                    int getClassVersion(int factoryId, int classId);

                    void setClassVersion(int factoryId, int classId, int version);

                    std::shared_ptr<ClassDefinition> lookupClassDefinition(int factoryId, int classId, int version);

                    std::shared_ptr<ClassDefinition> registerClassDefinition(std::shared_ptr<ClassDefinition>);

                    template<typename T>
                    std::shared_ptr<ClassDefinition> lookupOrRegisterClassDefinition(const T &portable);

                    int getVersion();

                    std::shared_ptr<ClassDefinition> readClassDefinition(ObjectDataInput &input, int id, int classId,
                                                                         int version);

                    const SerializationConfig &getSerializationConfig() const;

                    template<typename T>
                    typename std::enable_if<std::is_same<byte, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_BYTE; }

                    template<typename T>
                    typename std::enable_if<std::is_same<char, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_CHAR; }


                    template<typename T>
                    typename std::enable_if<std::is_same<char16_t, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_CHAR; }

                    template<typename T>
                    typename std::enable_if<std::is_same<bool, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_BOOLEAN; }

                    template<typename T>
                    typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_SHORT; }

                    template<typename T>
                    typename std::enable_if<std::is_same<int32_t, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_INT; }

                    template<typename T>
                    typename std::enable_if<std::is_same<int64_t, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_LONG; }

                    template<typename T>
                    typename std::enable_if<std::is_same<float, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_FLOAT; }

                    template<typename T>
                    typename std::enable_if<std::is_same<double, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_DOUBLE; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::string, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_UTF; }


                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<byte>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_BYTE_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<char>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_CHAR_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<bool>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_BOOLEAN_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<int16_t>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_SHORT_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<int32_t>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_INT_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<int64_t>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_LONG_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<float>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_FLOAT_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<double>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_DOUBLE_ARRAY; }

                    template<typename T>
                    typename std::enable_if<std::is_same<std::vector<std::string>, typename std::remove_cv<T>::type>::value, FieldType>::type
                    static getType() { return FieldType::TYPE_UTF_ARRAY; }

                private:
                    PortableContext(const PortableContext &) = delete;

                    ClassDefinitionContext &getClassDefinitionContext(int factoryId);

                    void operator=(const PortableContext &) = delete;

                    util::SynchronizedMap<int, ClassDefinitionContext> class_def_context_map_;
                    const SerializationConfig &serialization_config_;
                };

                class ClassDefinitionContext {
                public:

                    ClassDefinitionContext(int portableContext, PortableContext *pContext);

                    int getClassVersion(int classId);

                    void setClassVersion(int classId, int version);

                    std::shared_ptr<ClassDefinition> lookup(int, int);

                    std::shared_ptr<ClassDefinition> registerClassDefinition(std::shared_ptr<ClassDefinition>);

                private:
                    int64_t combineToLong(int x, int y) const;

                    const int factory_id_;
                    util::SynchronizedMap<long long, ClassDefinition> versioned_definitions_;
                    util::SynchronizedMap<int, int> current_class_versions_;
                    PortableContext *portable_context_;
                };

                class HAZELCAST_API ClassDefinitionWriter {
                public:
                    ClassDefinitionWriter(PortableContext &portableContext, ClassDefinitionBuilder &builder);

                    template <typename T>
                    void write(const std::string &fieldName, T value) {
                        typedef typename std::remove_pointer<typename std::remove_reference<typename std::remove_cv<T>::type>::type>::type value_type;
                        builder_.addField(fieldName, PortableContext::getType<value_type>());
                    }

                    template<typename T>
                    void writeNullPortable(const std::string &fieldName) {
                        T portable;
                        int32_t factoryId = hz_serializer<T>::getFactoryId();
                        int32_t classId = hz_serializer<T>::getClassId();
                        std::shared_ptr<ClassDefinition> nestedClassDef = context_.lookupClassDefinition(factoryId,
                                                                                                        classId,
                                                                                                        context_.getVersion());
                        if (!nestedClassDef) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("ClassDefWriter::writeNullPortable",
                                                                               "Cannot write null portable without explicitly registering class definition!"));
                        }
                        builder_.addPortableField(fieldName, nestedClassDef);
                    }

                    template<typename T>
                    void writePortable(const std::string &fieldName, const T *portable) {
                        if (NULL == portable) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("ClassDefinitionWriter::writePortable",
                                                                               "Cannot write null portable without explicitly registering class definition!"));
                        }

                        std::shared_ptr<ClassDefinition> nestedClassDef = createNestedClassDef(*portable);
                        builder_.addPortableField(fieldName, nestedClassDef);
                    };

                    template<typename T>
                    void writePortableArray(const std::string &fieldName, const std::vector<T> *portables) {
                        if (NULL == portables || portables->size() == 0) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ClassDefinitionWriter::writePortableArray",
                                                                  "Cannot write null portable array without explicitly registering class definition!"));
                        }
                        std::shared_ptr<ClassDefinition> nestedClassDef = createNestedClassDef((*portables)[0]);
                        builder_.addPortableArrayField(fieldName, nestedClassDef);
                    };

                    std::shared_ptr<ClassDefinition> registerAndGet();

                    ObjectDataOutput &getRawDataOutput();

                    void end();

                private:
                    template<typename T>
                    std::shared_ptr<ClassDefinition> createNestedClassDef(const T &portable);

                    ClassDefinitionBuilder &builder_;
                    PortableContext &context_;
                    ObjectDataOutput empty_data_output_;
                };

                class HAZELCAST_API PortableReaderBase {
                public:
                    PortableReaderBase(PortableSerializer &portableSer,
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
                        read(const std::string &fieldName) {
                        setPosition(fieldName, PortableContext::getType<T>());
                        return dataInput->read<T>();
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<boost::optional<std::string>, typename std::remove_cv<T>::type>::value, T>::type
                        read(const std::string &fieldName) {
                        setPosition(fieldName, PortableContext::getType<T>());
                        return dataInput->read<T>();
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
                                    read(const std::string &fieldName) {
                        setPosition(fieldName, PortableContext::getType<T>());
                        return dataInput->read<T>();
                    }

                    ObjectDataInput &getRawDataInput();

                    void end();

                protected:
                    void setPosition(const std::string &fieldName, FieldType const &fieldType);

                    void checkFactoryAndClass(FieldDefinition fd, int factoryId, int classId) const;

                    template<typename T>
                    boost::optional<T> getPortableInstance(const std::string &fieldName);

                    std::shared_ptr<ClassDefinition> cd;
                    ObjectDataInput *dataInput;
                    PortableSerializer *portableSerializer;
                private:
                    int final_position_;
                    int offset_;
                    bool raw_;

                };

                class HAZELCAST_API DefaultPortableReader : public PortableReaderBase {
                public:
                    DefaultPortableReader(PortableSerializer &portableSer,
                                          ObjectDataInput &input, std::shared_ptr<ClassDefinition> cd);

                    template<typename T>
                    boost::optional<T> readPortable(const std::string &fieldName);

                    template<typename T>
                    boost::optional<std::vector<T>> readPortableArray(const std::string &fieldName);
                };

                class HAZELCAST_API MorphingPortableReader : public PortableReaderBase {
                public:
                    MorphingPortableReader(PortableSerializer &portableSer, ObjectDataInput &input,
                                           std::shared_ptr<ClassDefinition> cd);

                    template <typename T>
                    typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<float, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<double, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &fieldName) {
                        if (!cd->hasField(fieldName)) {
                            return 0;
                        }
                        const FieldType &currentFieldType = cd->getFieldType(fieldName);
                        return readMorphing<T>(currentFieldType, fieldName);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<byte, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<char, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<char16_t, typename std::remove_cv<T>::type>::value ||
                                            std::is_same<bool, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &fieldName) {
                        if (!cd->hasField(fieldName)) {
                            return 0;
                        }
                        return PortableReaderBase::read<T>(fieldName);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<std::string, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &fieldName) {
                        if (!cd->hasField(fieldName)) {
                            return std::string();
                        }
                        return PortableReaderBase::read<T>(fieldName);
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<boost::optional<std::string>, typename std::remove_cv<T>::type>::value, T>::type
                    read(const std::string &fieldName) {
                        if (!cd->hasField(fieldName)) {
                            return boost::none;
                        }
                        return PortableReaderBase::read<T>(fieldName);
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
                    read(const std::string &fieldName) {
                        if (!cd->hasField(fieldName)) {
                            return boost::none;
                        }
                        return PortableReaderBase::read<T>(fieldName);
                    }

                    template<typename T>
                    boost::optional<T> readPortable(const std::string &fieldName);

                    template<typename T>
                    boost::optional<std::vector<T>> readPortableArray(const std::string &fieldName);

                private:
                    template <typename T>
                    typename std::enable_if<std::is_same<int16_t, typename std::remove_cv<T>::type>::value, T>::type
                    readMorphing(FieldType currentFieldType, const std::string &fieldName) {
                        switch(currentFieldType) {
                            case FieldType::TYPE_BYTE:
                                return PortableReaderBase::read<byte>(fieldName);
                            case FieldType::TYPE_SHORT:
                                return PortableReaderBase::read<int16_t>(fieldName);
                            default:
                                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                                 "IncompatibleClassChangeError"));
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<int32_t, typename std::remove_cv<T>::type>::value, T>::type
                    readMorphing(FieldType currentFieldType, const std::string &fieldName) {
                        switch(currentFieldType) {
                            case FieldType::TYPE_INT:
                                return PortableReaderBase::read<int32_t>(fieldName);
                            case FieldType::TYPE_CHAR:
                                return PortableReaderBase::read<char>(fieldName);
                            default:
                                return readMorphing<int16_t>(currentFieldType, fieldName);
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<int64_t, typename std::remove_cv<T>::type>::value, T>::type
                    readMorphing(FieldType currentFieldType, const std::string &fieldName) {
                        switch(currentFieldType) {
                            case FieldType::TYPE_LONG:
                                return PortableReaderBase::read<int64_t>(fieldName);
                            default:
                                return readMorphing<int32_t>(currentFieldType, fieldName);
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<float, typename std::remove_cv<T>::type>::value, T>::type
                    readMorphing(FieldType currentFieldType, const std::string &fieldName) {
                        switch(currentFieldType) {
                            case FieldType::TYPE_FLOAT:
                                return PortableReaderBase::read<float>(fieldName);
                            default:
                                return static_cast<float>(readMorphing<int32_t>(currentFieldType, fieldName));
                        }
                    }

                    template <typename T>
                    typename std::enable_if<std::is_same<double, typename std::remove_cv<T>::type>::value, T>::type
                    readMorphing(FieldType currentFieldType, const std::string &fieldName) {
                        switch(currentFieldType) {
                            case FieldType::TYPE_DOUBLE:
                                return PortableReaderBase::read<double>(fieldName);
                            case FieldType::TYPE_FLOAT:
                                return PortableReaderBase::read<float>(fieldName);
                            default:
                                return static_cast<double>(readMorphing<int64_t>(currentFieldType, fieldName));
                        }
                    }
                };

                class DefaultPortableWriter;
                class HAZELCAST_API PortableSerializer {
                    friend DefaultPortableWriter;
                public:
                    PortableSerializer(PortableContext &portableContext);

                    template<typename T>
                    T readObject(ObjectDataInput &in);

                    template<typename T>
                    T read(ObjectDataInput &in, int32_t factoryId, int32_t classId);

                    template<typename T>
                    void write(const T &object, ObjectDataOutput &out);

                private:
                    PortableContext &context_;

                    template<typename T>
                    int findPortableVersion(int factoryId, int classId) const;

                    PortableReader createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                int portableVersion);

                    int32_t readInt(ObjectDataInput &in) const;

                    template<typename T>
                    void writeInternal(const T &object, ObjectDataOutput &out);

                    template<typename T>
                    void writeInternal(const T &object, std::shared_ptr<ClassDefinition> &cd, ObjectDataOutput &out);

                    template<typename T>
                    std::shared_ptr<ClassDefinition> lookupOrRegisterClassDefinition(const T &portable);
                };

                class HAZELCAST_API DataSerializer {
                public:
                    template<typename T>
                    static boost::optional<T> readObject(ObjectDataInput &in) {
                        bool identified = in.read<bool>();
                        if (!identified) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ObjectDataInput::readObject<identified_data_serializer>",
                                                                  "Received data is not identified data serialized."));
                        }

                        int32_t expectedFactoryId = hz_serializer<T>::getFactoryId();
                        int32_t expectedClassId = hz_serializer<T>::getClassId();
                        int32_t factoryId = in.read<int32_t>();
                        int32_t classId = in.read<int32_t>();
                        if (expectedFactoryId != factoryId || expectedClassId != classId) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ObjectDataInput::readObject<identified_data_serializer>",
                                                                  (boost::format(
                                                                          "Factory id %1% and class id %2% of data do not match expected "
                                                                          "factory id %3% and class id %4%!") %
                                                                   factoryId % classId % expectedFactoryId %
                                                                   expectedClassId).str())
                            );
                        }

                        return boost::make_optional(hz_serializer<T>::readData(in));
                    }

                    template<typename T>
                    static void write(const T &object, ObjectDataOutput &out);

                private:
                    int32_t readInt(ObjectDataInput &in) const;
                };

                class HAZELCAST_API DefaultPortableWriter {
                public:
                    DefaultPortableWriter(PortableSerializer &portableSer, std::shared_ptr<ClassDefinition> cd,
                                          ObjectDataOutput &output);

                    ObjectDataOutput &getRawDataOutput();

                    template <typename T>
                    void write(const std::string &fieldName, T value) {
                        typedef typename std::remove_pointer<typename std::remove_reference<typename std::remove_cv<T>::type>::type>::type value_type;
                        setPosition(fieldName, PortableContext::getType<value_type>());
                        object_data_output_.write(value);
                    }

                    void end();

                    template<typename T>
                    void writeNullPortable(const std::string &fieldName);

                    template<typename T>
                    void writePortable(const std::string &fieldName, const T *portable);

                    template<typename T>
                    void writePortableArray(const std::string &fieldName, const std::vector<T> *values);

                private:
                    FieldDefinition const &setPosition(const std::string &fieldName, FieldType fieldType);

                    template<typename T>
                    void checkPortableAttributes(const FieldDefinition &fd);

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

                    PortableSerializer &getPortableSerializer();

                    DataSerializer &getDataSerializer();

                    template<typename T>
                    inline Data toData(const T *object) {
                        ObjectDataOutput output(false, &portable_serializer_, serialization_config_.getGlobalSerializer());

                        writeHash<T>(object, output);

                        output.writeObject<T>(object);

                        return {std::move(output).toByteArray()};
                    }

                    template<typename T>
                    inline Data toData(const T &object) {
                        ObjectDataOutput output(false, &portable_serializer_, serialization_config_.getGlobalSerializer());

                        writeHash<T>(&object, output);

                        output.writeObject<T>(object);

                        return {std::move(output).toByteArray()};
                    }

                    template<typename T>
                    inline std::shared_ptr<Data> toSharedData(const T *object) {
                        if (NULL == object) {
                            return std::shared_ptr<Data>();
                        }
                        return std::shared_ptr<Data>(new Data(toData<T>(object)));
                    }

                    template<typename T>
                    inline boost::optional<T> toObject(const Data *data) {
                        if (!data) {
                            return boost::none;
                        }
                        return toObject<T>(*data);
                    }

                    template<typename T>
                    typename std::enable_if<!(std::is_same<T, const char *>::value ||
                                              std::is_same<T, const char *>::value ||
                                              std::is_same<T, TypedData>::value), boost::optional<T>>::type
                    inline toObject(const Data &data) {
                        if (isNullData(data)) {
                            return boost::none;
                        }

                        int32_t typeId = data.getType();

                        // Constant 8 is Data::DATA_OFFSET. Windows DLL export does not
                        // let usage of static member.
                        ObjectDataInput objectDataInput(data.toByteArray(), 8, portable_serializer_, data_serializer_,
                                                        serialization_config_.getGlobalSerializer());
                        return objectDataInput.readObject<T>(typeId);
                    }

                    template<typename T>
                    typename std::enable_if<std::is_same<T, TypedData>::value, boost::optional<T>>::type
                    inline toObject(const Data &data) {
                        return boost::make_optional(TypedData(Data(data), *this));
                    }

                    template<typename T>
                    typename std::enable_if<std::is_same<T, const char *>::value, boost::optional<std::string>>::type
                    inline toObject(const Data &data) {
                        return toObject<std::string>(data);
                    }

                    template<typename T>
                    typename std::enable_if<std::is_array<T>::value &&
                                            std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
                    inline toObject(const Data &data) {
                        return toObject<std::string>(data);
                    }

                    template<typename T>
                    inline std::shared_ptr<Data> toSharedObject(const std::shared_ptr<Data> &data) {
                        return data;
                    }

                    const byte getVersion() const;

                    ObjectType getObjectType(const Data *data);

                    /**
                     * @link Disposable interface implementation
                     */
                    void dispose() override;

                    ObjectDataOutput newOutputStream();
                private:
                    SerializationService(const SerializationService &) = delete;

                    SerializationService &operator=(const SerializationService &) = delete;

                    const SerializationConfig &serialization_config_;
                    PortableContext portable_context_;
                    serialization::pimpl::PortableSerializer portable_serializer_;
                    serialization::pimpl::DataSerializer data_serializer_;

                    static bool isNullData(const Data &data);

                    template<typename T>
                    void writeHash(const PartitionAwareMarker *obj, DataOutput &out) {
                        typedef typename T::KEY_TYPE PK_TYPE;
                        const PartitionAware<PK_TYPE> *partitionAwareObj = static_cast<const PartitionAware<PK_TYPE> *>(obj);
                        const PK_TYPE *pk = partitionAwareObj->getPartitionKey();
                        if (pk != NULL) {
                            Data partitionKey = toData<PK_TYPE>(pk);
                            out.write<int32_t>(partitionKey.getPartitionHash());
                        }
                    }

                    template<typename T>
                    void writeHash(const void *obj, DataOutput &out) {
                        out.write<int32_t>(0);
                    }
                };

                template<>
                Data HAZELCAST_API SerializationService::toData(const char *object);
            }

            /**
            * Provides a mean of reading portable fields from a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableReader {
            public:
                PortableReader(pimpl::PortableSerializer &portableSer, ObjectDataInput &dataInput,
                               const std::shared_ptr<ClassDefinition>& cd, bool isDefaultReader);

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
                read(const std::string &fieldName) {
                    if (is_default_reader_)
                        return default_portable_reader_->read<T>(fieldName);
                    return morphing_portable_reader_->read<T>(fieldName);
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
                read(const std::string &fieldName) {
                    if (is_default_reader_)
                        return default_portable_reader_->read<T>(fieldName);
                    return morphing_portable_reader_->read<T>(fieldName);
                }

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @return the portable value read
                */
                template<typename T>
                boost::optional<T> readPortable(const std::string &fieldName);

                /**
                * @tparam type of the portable class in array
                * @param fieldName name of the field
                * @return the portable array value read
                */
                template<typename T>
                boost::optional<std::vector<T>> readPortableArray(const std::string &fieldName);

                /**
                * @see PortableWriter#getRawDataOutput
                *
                * Note that portable fields can not read after getRawDataInput() is called. In case this happens,
                * IOException will be thrown.
                *
                * @return rawDataInput
                * @throws IOException
                */
                ObjectDataInput &getRawDataInput();

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
                PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter);

                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter);

                template <typename T>
                void write(const std::string &fieldName, T value) {
                    if(is_default_writer_) {
                        default_portable_writer_->write(fieldName, value);
                    } else {
                        class_definition_writer_->write(fieldName, value);
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
                void writeNullPortable(const std::string &fieldName);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param portable  Portable to be written
                * @throws IOException
                */
                template<typename T>
                void writePortable(const std::string &fieldName, const T *portable);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param values portable array to be written
                * @throws IOException
                */
                template<typename T>
                void writePortableArray(const std::string &fieldName, const std::vector<T> *values);

                /**
                * After writing portable fields, one can write remaining fields in old fashioned way consecutively at the end
                * of stream. User should not that after getting rawDataOutput trying to write portable fields will result
                * in IOException
                *
                * @return ObjectDataOutput
                * @throws IOException
                */
                ObjectDataOutput &getRawDataOutput();

            private:
                pimpl::DefaultPortableWriter *default_portable_writer_;
                pimpl::ClassDefinitionWriter *class_definition_writer_;
                bool is_default_writer_;
            };

            template<typename T>
            boost::optional<T> PortableReader::readPortable(const std::string &fieldName) {
                if (is_default_reader_)
                    return default_portable_reader_->readPortable<T>(fieldName);
                return morphing_portable_reader_->readPortable<T>(fieldName);
            }

            /**
            * @tparam type of the portable class in array
            * @param fieldName name of the field
            * @return the portable array value read
            * @throws IOException
            */
            template<typename T>
            boost::optional<std::vector<T>> PortableReader::readPortableArray(const std::string &fieldName) {
                if (is_default_reader_)
                    return default_portable_reader_->readPortableArray<T>(fieldName);
                return morphing_portable_reader_->readPortableArray<T>(fieldName);
            };

            template<typename T>
            void PortableWriter::writeNullPortable(const std::string &fieldName) {
                if (is_default_writer_)
                    return default_portable_writer_->writeNullPortable<T>(fieldName);
                return class_definition_writer_->writeNullPortable<T>(fieldName);
            }

            /**
            * @tparam type of the portable class
            * @param fieldName name of the field
            * @param portable  Portable to be written
            * @throws IOException
            */
            template<typename T>
            void PortableWriter::writePortable(const std::string &fieldName, const T *portable) {
                if (is_default_writer_)
                    return default_portable_writer_->writePortable(fieldName, portable);
                return class_definition_writer_->writePortable(fieldName, portable);

            }

            /**
            * @tparam type of the portable class
            * @param fieldName name of the field
            * @param values portable array to be written
            * @throws IOException
            */
            template<typename T>
            void PortableWriter::writePortableArray(const std::string &fieldName, const std::vector<T> *values) {
                if (is_default_writer_)
                    return default_portable_writer_->writePortableArray(fieldName, values);
                return class_definition_writer_->writePortableArray(fieldName, values);
            }

            template<typename T>
            void ObjectDataOutput::writeObject(const T *object) {
                if (isNoWrite) { return; }
                if (!object) {
                    write<int32_t>(static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_NULL));
                    return;
                }

                writeObject<T>(*object);
            }

            template<typename T>
            typename std::enable_if<!(std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value), void>::type
            ObjectDataOutput::writeObject(const boost::optional<T> &object) {
                if (isNoWrite) { return; }
                if (!object) {
                    write<int32_t>(static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_NULL));
                    return;
                }

                writeObject<T>(object.value());
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                if (isNoWrite) { return; }
                write<int32_t>(static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_DATA));
                pimpl::DataSerializer::write<T>(object, *this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                if (isNoWrite) { return; }
                write<int32_t>(static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE));
                portable_serializer_->write<T>(object, *this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                if (isNoWrite) { return; }
                write<int32_t>(static_cast<int32_t>((hz_serializer<T>::getTypeId())));
                write < T > (object);
            }

            template<typename T>
            typename std::enable_if<std::is_array<T>::value && std::is_same<typename std::remove_all_extents<T>::type, char>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                writeObject(std::string(object));
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                if (isNoWrite) { return; }
                static_assert(hz_serializer<T>::getTypeId() > 0, "Custom serializer type id can not be negative!");
                write<int32_t>(hz_serializer<T>::getTypeId());
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
            inline ObjectDataOutput::writeObject(const T object) {
                if (!global_serializer_) {
                    throw exception::HazelcastSerializationException("ObjectDataOutput::writeObject",
                            (boost::format("No serializer found for type(%1%).") %typeid(T).name()).str());
                }
                if (isNoWrite) { return; }
                write<int32_t>(static_cast<int32_t>(global_serializer::getTypeId()));
                global_serializer_->write(boost::any(std::move(object)), *this);
            }

            template<typename T>
            typename std::enable_if<!(std::is_array<T>::value &&
                                      std::is_same<typename std::remove_all_extents<T>::type, char>::value), boost::optional<T>>::type
            inline ObjectDataInput::readObject() {
                int32_t typeId = read<int32_t>();
                if (static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_NULL) == typeId) {
                    return boost::none;
                }
                return readObject<T>(typeId);
            }

            template<typename T>
            typename std::enable_if<std::is_array<T>::value &&
                                    std::is_same<typename std::remove_all_extents<T>::type, char>::value, boost::optional<std::string>>::type
            inline ObjectDataInput::readObject() {
                return readObject<std::string>();
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                if (typeId != static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_DATA)) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                  "ObjectDataInput::readObject<identified_data_serializer>",
                                                          (boost::format(
                                                                  "The associated serializer Serializer<T> is identified_data_serializer "
                                                                  "but received data type id is %1%") % typeId).str()));
                }

                return data_serializer_.readObject<T>(*this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                if (typeId != static_cast<int32_t>(pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE)) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                  "ObjectDataInput::readObject<portable_serializer>",
                                                          (boost::format(
                                                                  "The associated serializer Serializer<T> is portable_serializer "
                                                                  "but received data type id is %1%") % typeId).str()));
                }

                return portable_serializer_.readObject<T>(*this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<custom_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                if (typeId != hz_serializer<T>::getTypeId()) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ObjectDataInput::readObject<>",
                                                                                     (boost::format(
                                                                                             "The associated serializer Serializer<T> type id %1% does not match "
                                                                                             "received data type id is %2%") %
                                                                                      hz_serializer<T>::getTypeId() %
                                                                                      typeId).str()));
                }

                return boost::optional<T>(hz_serializer<T>::read(*this));
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<builtin_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                assert(typeId == static_cast<int32_t>(hz_serializer<T>::getTypeId()));

                return boost::optional<T>(read<T>());
            }

            template<typename T>
            typename std::enable_if<!(std::is_base_of<identified_data_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<portable_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<builtin_serializer, hz_serializer<T>>::value ||
                                      std::is_base_of<custom_serializer, hz_serializer<T>>::value), boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                if (!global_serializer_) {
                    throw exception::HazelcastSerializationException("ObjectDataInput::readObject",
                            (boost::format("No serializer found for type %1%.") %typeid(T).name()).str());
                }

                if (typeId != static_cast<int32_t>(global_serializer_->getTypeId())) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ObjectDataInput::readObject<>",
                                                                                     (boost::format(
                                                                                             "The global serializer type id %1% does not match "
                                                                                             "received data type id is %2%") %
                                                                                             static_cast<int32_t>(global_serializer_->getTypeId()) %
                                                                                      typeId).str()));
                }

                return boost::optional<T>(boost::any_cast<T>(std::move(global_serializer_->read(*this))));
            }

            namespace pimpl {
                template<>
                Data SerializationService::toData(const TypedData *object);

                template<typename T>
                boost::optional<T> DefaultPortableReader::readPortable(const std::string &fieldName) {
                    return getPortableInstance<T>(fieldName);
                }

                template<typename T>
                boost::optional<std::vector<T>> DefaultPortableReader::readPortableArray(const std::string &fieldName) {
                    PortableReaderBase::setPosition(fieldName, FieldType::TYPE_PORTABLE_ARRAY);

                    dataInput->read<int32_t>();
                    std::vector<T> portables;

                    setPosition(fieldName, FieldType::TYPE_PORTABLE_ARRAY);

                    int32_t len = dataInput->read<int32_t>();
                    if (len == util::Bits::NULL_ARRAY) {
                        return boost::none;
                    }
                    int32_t factoryId = dataInput->read<int32_t>();
                    int32_t classId = dataInput->read<int32_t>();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (len > 0) {
                        int offset = dataInput->position();
                        for (int i = 0; i < len; i++) {
                            dataInput->position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int32_t start = dataInput->read<int32_t>();
                            dataInput->position(start);

                            portables.push_back(portableSerializer->read<T>(*dataInput, factoryId, classId));
                        }
                    }
                    return portables;
                }

                template<typename T>
                boost::optional<T> MorphingPortableReader::readPortable(const std::string &fieldName) {
                    return getPortableInstance<T>(fieldName);
                }

                template<typename T>
                boost::optional<std::vector<T>> MorphingPortableReader::readPortableArray(const std::string &fieldName) {
                    PortableReaderBase::setPosition(fieldName, FieldType::TYPE_PORTABLE_ARRAY);

                    dataInput->read<int32_t>();
                    std::vector<T> portables;

                    setPosition(fieldName, FieldType::TYPE_PORTABLE_ARRAY);

                    int32_t len = dataInput->read<int32_t>();
                    if (len == util::Bits::NULL_ARRAY) {
                        return boost::none;
                    }
                    int32_t factoryId = dataInput->read<int32_t>();
                    int32_t classId = dataInput->read<int32_t>();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (len > 0) {
                        portables.reserve(static_cast<size_t>(len));
                        int offset = dataInput->position();
                        for (int i = 0; i < len; i++) {
                            dataInput->position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int32_t start = dataInput->read<int32_t>();
                            dataInput->position(start);

                            portables.emplace_back(portableSerializer->read<T>(*dataInput, factoryId, classId));
                        }
                    }

                    return boost::make_optional(std::move(portables));
                }

                template<typename T>
                T PortableSerializer::readObject(ObjectDataInput &in) {
                    int32_t factoryId = readInt(in);
                    int32_t classId = readInt(in);

                    if (factoryId != hz_serializer<T>::getFactoryId() || classId != hz_serializer<T>::getClassId()) {
                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastSerializationException("PortableSerializer::readObject",
                                                                           (boost::format("Received data (factory-class id)=(%1%, %2%) does not match expected factory-class id (%3%, %4%)") %
                                                                            factoryId % classId %
                                                                            hz_serializer<T>::getFactoryId() %
                                                                            hz_serializer<T>::getClassId()).str()));
                    }
                    return read<T>(in, factoryId, classId);
                }

                template<typename T>
                T PortableSerializer::read(ObjectDataInput &in, int32_t factoryId, int32_t classId) {
                    int version = in.read<int32_t>();

                    int portableVersion = findPortableVersion<T>(factoryId, classId);

                    PortableReader reader = createReader(in, factoryId, classId, version, portableVersion);
                    T result = hz_serializer<T>::readPortable(reader);
                    reader.end();
                    return result;
                }

                template<typename T>
                void PortableSerializer::write(const T &object, ObjectDataOutput &out) {
                    out.write<int32_t>(hz_serializer<T>::getFactoryId());
                    out.write<int32_t>(hz_serializer<T>::getClassId());

                    writeInternal(object, out);
                }

                template<typename T>
                void PortableSerializer::writeInternal(const T &object, ObjectDataOutput &out) {
                    auto cd = context_.lookupOrRegisterClassDefinition<T>(object);
                    writeInternal(object, cd, out);
                }

                template<typename T>
                void PortableSerializer::writeInternal(const T &object, std::shared_ptr<ClassDefinition> &cd,
                                               ObjectDataOutput &out) {
                    out.write<int32_t>(cd->getVersion());

                    DefaultPortableWriter dpw(*this, cd, out);
                    PortableWriter portableWriter(&dpw);
                    hz_serializer<T>::writePortable(object, portableWriter);
                    portableWriter.end();
                }

                template<typename T>
                std::shared_ptr<ClassDefinition> PortableSerializer::lookupOrRegisterClassDefinition(const T &portable) {
                    return context_.lookupOrRegisterClassDefinition<T>(portable);
                }

                template<typename T>
                int PortableSerializer::findPortableVersion(int factoryId, int classId) const {
                    int currentVersion = context_.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion<T>(context_.getVersion());
                        if (currentVersion > 0) {
                            context_.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                template<typename T>
                void DataSerializer::write(const T &object, ObjectDataOutput &out) {
                    out.write<bool>(true);
                    out.write<int32_t>(hz_serializer<T>::getFactoryId());
                    out.write<int32_t>(hz_serializer<T>::getClassId());
                    hz_serializer<T>::writeData(object, out);
                }

                template<typename T>
                std::shared_ptr<ClassDefinition>
                PortableContext::lookupOrRegisterClassDefinition(const T &portable) {
                    int portableVersion = PortableVersionHelper::getVersion<T>(
                            serialization_config_.getPortableVersion());
                    std::shared_ptr<ClassDefinition> cd = lookupClassDefinition(hz_serializer<T>::getFactoryId(),
                                                                                hz_serializer<T>::getClassId(),
                                                                                portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(hz_serializer<T>::getFactoryId(),
                                                                      hz_serializer<T>::getClassId(), portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        hz_serializer<T>::writePortable(portable, portableWriter);
                        cd = cdw.registerAndGet();
                    }
                    return cd;
                }

                template<typename T>
                boost::optional<T> PortableReaderBase::getPortableInstance(const std::string &fieldName) {
                    setPosition(fieldName, FieldType::TYPE_PORTABLE);

                    bool isNull = dataInput->read<bool>();
                    int32_t factoryId = dataInput->read<int32_t>();
                    int32_t classId = dataInput->read<int32_t>();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (isNull) {
                        return boost::none;
                    } else {
                        return portableSerializer->read<T>(*dataInput, factoryId, classId);
                    }
                }

                template<typename T>
                void DefaultPortableWriter::writeNullPortable(const std::string &fieldName) {
                    setPosition(fieldName, FieldType::TYPE_PORTABLE);
                    object_data_output_.write<bool>(true);
                    object_data_output_.write<int32_t>(hz_serializer<T>::getFactoryId());
                    object_data_output_.write<int32_t>(hz_serializer<T>::getClassId());
                }

                template<typename T>
                void DefaultPortableWriter::writePortable(const std::string &fieldName, const T *portable) {
                    FieldDefinition const &fd = setPosition(fieldName, FieldType::TYPE_PORTABLE);
                    bool isNull = (nullptr == portable);
                    object_data_output_.write<bool>(isNull);

                    object_data_output_.write<int32_t>(hz_serializer<T>::getFactoryId());
                    object_data_output_.write<int32_t>(hz_serializer<T>::getClassId());

                    if (!isNull) {
                        checkPortableAttributes<T>(fd);
                        portable_serializer_.writeInternal(*portable, object_data_output_);
                    }

                    portable_serializer_.write(*portable, object_data_output_);
                }

                template<typename T>
                void DefaultPortableWriter::writePortableArray(const std::string &fieldName, const std::vector<T> *values) {
                    FieldDefinition const &fd = setPosition(fieldName, FieldType::TYPE_PORTABLE_ARRAY);
                    checkPortableAttributes<T>(fd);

                    int32_t len = (values ? static_cast<int32_t>(values->size()) : util::Bits::NULL_ARRAY);
                    object_data_output_.write<int32_t>(len);

                    object_data_output_.write<int32_t>(fd.getFactoryId());
                    object_data_output_.write<int32_t>(fd.getClassId());

                    if (len > 0) {
                        std::shared_ptr<ClassDefinition> classDefinition = portable_serializer_.lookupOrRegisterClassDefinition<T>(
                                (*values)[0]);
                        size_t currentOffset = object_data_output_.position();
                        object_data_output_.position(currentOffset + len * util::Bits::INT_SIZE_IN_BYTES);
                        for (int32_t i = 0; i < len; i++) {
                            size_t position = object_data_output_.position();
                            object_data_output_.writeAt(currentOffset + i * util::Bits::INT_SIZE_IN_BYTES, static_cast<int32_t>(position));
                            portable_serializer_.writeInternal((*values)[i], classDefinition, object_data_output_);
                        }
                    }
                }

                template<typename T>
                void DefaultPortableWriter::checkPortableAttributes(const FieldDefinition &fd) {
                    if (fd.getFactoryId() != hz_serializer<T>::getFactoryId()) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", (boost::format(
                                                              "Wrong Portable type! Expected factory-id: %1%, Actual factory-id: %2%")
                                                              %fd.getFactoryId() %hz_serializer<T>::getFactoryId()).str()));
                    }
                    if (fd.getClassId() != hz_serializer<T>::getClassId()) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", (boost::format(
                                                              "Wrong Portable type! Expected class-id: %1%, Actual class-id: %2%")
                                                              %fd.getClassId() %hz_serializer<T>::getClassId()).str()));
                    }
                }

                template<typename T>
                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::createNestedClassDef(const T &portable) {
                    int version = PortableVersionHelper::getVersion<T>(context_.getVersion());
                    ClassDefinitionBuilder definitionBuilder(hz_serializer<T>::getFactoryId(), hz_serializer<T>::getClassId(),
                                                             version);

                    ClassDefinitionWriter nestedWriter(context_, definitionBuilder);
                    PortableWriter portableWriter(&nestedWriter);
                    hz_serializer<T>::writePortable(portable, portableWriter);
                    return context_.registerClassDefinition(definitionBuilder.build());
                }
            }
        }

        template <typename T>
        boost::optional<T> TypedData::get() const {
            return ss_->toObject<T>(data_);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

