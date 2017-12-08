/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 8/12/13.

#ifndef HAZELCAST_ObjectDataOutput
#define HAZELCAST_ObjectDataOutput

#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/TypeIDS.h"
#include "hazelcast/client/PartitionAware.h"

#include<stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class DataOutput;

                class Data;
            }

            /**
            * Provides serialization methods for primitive types,a arrays of primitive types, Portable,
            * IdentifiedDataSerializable and custom serializables.
            * For custom serialization @see Serializer
            */
            class HAZELCAST_API ObjectDataOutput {
            public:
                /**
                * Internal API Constructor
                */
                ObjectDataOutput(pimpl::DataOutput& dataOutput, pimpl::SerializerHolder *serializerHolder);

                /**
                * Internal API Constructor
                */
                ObjectDataOutput();

                /**
                * @return copy of internal byte array
                */
                std::auto_ptr<std::vector<byte> > toByteArray();

                /**
                * Writes all the bytes in array to stream
                * @param bytes to be written
                */
                void write(const std::vector<byte>& bytes);

                /**
                * @param value the bool value to be written
                */
                void writeBoolean(bool value);

                /**
                * @param value the byte value to be written
                */
                void writeByte(int32_t value);

                /**
                 * @param bytes The data bytes to be written
                 * @param len Number of bytes to write
                 */
                void writeBytes(const byte *bytes, size_t len);

                /**
                * @param value the int16_t value to be written
                */
                void writeShort(int32_t value);

                /**
                * @param value the char value to be written
                */
                void writeChar(int32_t value);

                /**
                * @param value the int32_t value to be written
                */
                void writeInt(int32_t value);

                /**
                * @param value the int64_t  value to be written
                */
                void writeLong(int64_t value);

                /**
                * @param value the float value to be written
                */
                void writeFloat(float value);

                /**
                * @param value the double value to be written
                */
                void writeDouble(double value);

                /**
                * @param value the UTF string value to be written
                */
                void writeUTF(const std::string *value);

                /**
                * @param value the bytes to be written
                */
                void writeByteArray(const std::vector<byte> *value);

                /**
                * @param value the characters to be written
                */
                void writeCharArray(const std::vector<char> *value);

                /**
                * @param value the boolean values to be written
                */
                void writeBooleanArray(const std::vector<bool> *value);

                /**
                * @param value the int16_t array value to be written
                */
                void writeShortArray(const std::vector<int16_t> *value);

                /**
                * @param value the int32_t array value to be written
                */
                void writeIntArray(const std::vector<int32_t> *value);

                /**
                * @param value the int16_t array value to be written
                */
                void writeLongArray(const std::vector<int64_t > *value);

                /**
                * @param value the float array value to be written
                */
                void writeFloatArray(const std::vector<float> *value);

                /**
                * @param value the double array value to be written
                */
                void writeDoubleArray(const std::vector<double> *value);

                /**
                 * @param strings the array of strings to be serialized
                 */
                void writeUTFArray(const std::vector<std::string *> *strings);

                /**
                * @param value the data value to be written
                */
                void writeData(const pimpl::Data *value);

                /**
                * @param object serializable object to be written
                * @see Serializer
                * @throws IOException
                */
                template <typename T>
                void writeObject(const T *object) {
                    if (isEmpty) return;

                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        int32_t type = getHazelcastTypeId(object);

                        boost::shared_ptr<SerializerBase> serializer = serializerHolder->serializerFor(type);

                        if (NULL == serializer.get()) {
                            const std::string message = "No serializer found for serializerId :"+
                                                         util::IOUtil::to_string(type) + ", typename :" +
                                                         typeid(T).name();
                            throw exception::HazelcastSerializationException("ObjectDataOutput::toData", message);
                        }

                        boost::shared_ptr<StreamSerializer> streamSerializer = boost::static_pointer_cast<StreamSerializer>(
                                serializer);

                        writeInt(streamSerializer->getHazelcastTypeId());

                        writeInternal<T>(object, streamSerializer);
                    }
                }

                pimpl::DataOutput *getDataOutput() const;

            private:
                pimpl::DataOutput *dataOutput;
                pimpl::PortableContext *context;
                pimpl::SerializerHolder *serializerHolder;
                bool isEmpty;

                size_t position();

                void position(size_t newPos);

                ObjectDataOutput(const ObjectDataOutput&);

                void operator=(const ObjectDataOutput&);

                /**
                 * This method is written for backward compatibility of std::vector<std::string> type handling.
                 * @tparam T The type
                 * @param out The data output
                 * @param object The object to be written
                 * @param streamSerializer The serializer to be used
                 */
                template <typename T>
                void writeInternal(const T *object,
                                   boost::shared_ptr<StreamSerializer> &streamSerializer) {
                    int32_t typeId = streamSerializer->getHazelcastTypeId();
                    switch (typeId) {
                        case serialization::pimpl::SerializationConstants::CONSTANT_TYPE_DATA: {
                            serialization::pimpl::DataSerializer *dataSerializer =
                                    static_cast<serialization::pimpl::DataSerializer *>(streamSerializer.get());
                            return dataSerializer->write(*this, object);
                        }
                        case serialization::pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE: {
                            serialization::pimpl::PortableSerializer *portableSerializer =
                                    static_cast<serialization::pimpl::PortableSerializer *>(streamSerializer.get());

                            return portableSerializer->write(*this, object);
                        }
                        default: {
                            streamSerializer->write(*this, object);
                        }
                    }
                }
            };

            template <>
            HAZELCAST_API void ObjectDataOutput::writeInternal(const std::vector<std::string> *object,
                               boost::shared_ptr<StreamSerializer> &streamSerializer);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ObjectDataOutput

