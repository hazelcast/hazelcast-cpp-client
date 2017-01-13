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
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/TypeIDS.h"

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
                ObjectDataOutput(pimpl::DataOutput& dataOutput, pimpl::PortableContext& portableContext);

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
                void writeByte(int value);

                /**
                 * @param bytes The data bytes to be written
                 * @param len Number of bytes to write
                 */
                void writeBytes(const byte *bytes, unsigned int len);

                /**
                * @param value the short value to be written
                */
                void writeShort(int value);

                /**
                * @param value the char value to be written
                */
                void writeChar(int value);

                /**
                * @param value the int value to be written
                */
                void writeInt(int value);

                /**
                * @param value the long value to be written
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
                * @param value the short array value to be written
                */
                void writeShortArray(const std::vector<short> *value);

                /**
                * @param value the int array value to be written
                */
                void writeIntArray(const std::vector<int> *value);

                /**
                * @param value the short array value to be written
                */
                void writeLongArray(const std::vector<long> *value);

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
                * @param object Portable object to be written
                * @see Portable
                * @throws IOException
                */
                template <typename T>
                void writeObject(const Portable *object) {
                    if (isEmpty) return;

                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE);

                        writeInt(object->getFactoryId());
                        writeInt(object->getClassId());

                        context->getSerializerHolder().getPortableSerializer().write(*this->dataOutput, *object);
                    }
                }

                /**
                * @param object IdentifiedDataSerializable object to be written
                * @see IdentifiedDataSerializable
                * @throws IOException
                */
                template<typename T>
                void writeObject(const IdentifiedDataSerializable *object) {
                    if (isEmpty) return;

                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_DATA);
                        context->getSerializerHolder().getDataSerializer().write(*this, *object);
                    }
                }

                /**
                * @param object custom serializable object to be written
                * @see Serializer
                * @throws IOException
                */
                template <typename T>
                void writeObject(const void *serializable) {
                    if (isEmpty) return;

                    if (NULL == serializable) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        const T *object = static_cast<const T *>(serializable);
                        int type = getHazelcastTypeId(object);
                        writeInt(type);

                        boost::shared_ptr<SerializerBase> serializer = context->getSerializerHolder().serializerFor(type);

                        if (NULL == serializer.get()) {
                            const std::string message = "No serializer found for serializerId :"+
                                                         util::IOUtil::to_string(type) + ", typename :" +
                                                         typeid(T).name();
                            throw exception::HazelcastSerializationException("ObjectDataOutput::toData", message);
                        }

                        Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());

                        s->write(*this, *object);
                    }
                }

                template <typename T>
                void writeObject(const byte *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_BYTE);
                        writeByte(*object);
                    }
                }

                template <typename T>
                void writeObject(const bool *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN);
                        writeBoolean(*object);
                    }
                }

                template <typename T>
                void writeObject(const char *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_CHAR);
                        writeChar(*object);
                    }
                }

                template <typename T>
                void writeObject(const short *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_SHORT);
                        writeShort(*object);
                    }
                }

                template <typename T>
                void writeObject(const int *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER);
                        writeInt(*object);
                    }
                }

                template <typename T>
                void writeObject(const long *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_LONG);
                        writeLong(*object);
                    }
                }

                template <typename T>
                void writeObject(const float *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT);
                        writeFloat(*object);
                    }
                }

                template <typename T>
                void writeObject(const double *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE);
                        writeDouble(*object);
                    }
                }

                template <typename T>
                void writeObject(const std::string *object) {
                    if (NULL == object) {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    } else {
                        writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_STRING);
                        writeUTF(object);
                    }
                }
            private:
                pimpl::DataOutput *dataOutput;
                pimpl::PortableContext *context;
                pimpl::SerializerHolder *serializerHolder;
                bool isEmpty;

                size_t position();

                void position(size_t newPos);

                ObjectDataOutput(const ObjectDataOutput&);

                void operator=(const ObjectDataOutput&);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ObjectDataOutput

