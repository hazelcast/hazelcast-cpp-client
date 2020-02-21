/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//  ObjectDataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_INPUT
#define HAZELCAST_DATA_INPUT

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/pimpl/ConstantSerializers.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/TypeIDS.h"
#include "hazelcast/util/HazelcastDll.h"

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;

            class IdentifiedDataSerializable;

            namespace pimpl {
                class PortableContext;

                class DataInput;

                class Data;
            }

            /**
            * Provides deserialization methods for primitives types, arrays of primitive types
            * Portable, IdentifiedDataSerializable and custom serializable types
            */
            class HAZELCAST_API ObjectDataInput {
            public:
                /**
                * Internal API. Constructor
                */
                ObjectDataInput(pimpl::DataInput &dataInput, pimpl::SerializerHolder &serializerHolder);

                /**
                * fills all content to given byteArray
                * @param byteArray to fill the data in
                */
                void readFully(std::vector<byte> &byteArray);

                /**
                *
                * @param i number of bytes to skip
                */
                int skipBytes(int i);

                /**
                * @return the boolean read
                * @throws IOException if it reaches end of file before finish reading
                */
                bool readBoolean();

                /**
                * @return the byte read
                * @throws IOException if it reaches end of file before finish reading
                */
                byte readByte();

                /**
                * @return the int16_t read
                * @throws IOException if it reaches end of file before finish reading
                */
                int16_t readShort();

                /**
                * @return the char read
                * @throws IOException if it reaches end of file before finish reading
                */
                char readChar();

                /**
                * @return the int32_t read
                * @throws IOException if it reaches end of file before finish reading
                */
                int32_t readInt();

                /**
                * @return the int64_t read
                * @throws IOException if it reaches end of file before finish reading
                */
                int64_t readLong();

                /**
                * @return the boolean read
                * @throws IOException if it reaches end of file before finish reading
                */
                float readFloat();

                /**
                * @return the double read
                * @throws IOException if it reaches end of file before finish reading
                */
                double readDouble();

                /**
                * @return the utf string read as an ascii string
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::string> readUTF();

                /**
                * @return the byte array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<byte> > readByteArray();

                /**
                * @return the boolean array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<bool> > readBooleanArray();

                /**
                * @return the char array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<char> > readCharArray();

                /**
                * @return the int32_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<int32_t> > readIntArray();

                /**
                * @return the int64_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<int64_t> > readLongArray();

                /**
                * @return the double array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<double> > readDoubleArray();

                /**
                * @return the float array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<float> > readFloatArray();

                /**
                * @return the int16_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<int16_t> > readShortArray();

                /**
                * @return the array of strings
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<std::string> > readUTFArray();

                /**
                * @return the array of strings
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<std::string *> > readUTFPointerArray();

                /**
                * Object can be Portable, IdentifiedDataSerializable or custom serializable
                * for custom serialization @see Serializer
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                std::unique_ptr<T> readObject() {
                    int32_t typeId = readInt();
                    return readObject<T>(typeId);
                }

                template<typename T>
                std::unique_ptr<T> readObject(int32_t typeId) {
                    std::shared_ptr<SerializerBase> serializer = serializerHolder.serializerFor(typeId);
                    if (NULL == serializer.get()) {
                        const std::string message = "No serializer found for serializerId :" +
                                                    util::IOUtil::to_string(typeId) + ", typename :" +
                                                    typeid(T).name();
                        throw exception::HazelcastSerializationException("ObjectDataInput::readInternal", message);
                    }

                    return readObjectInternal<T>(typeId, serializer);
                }

                /**
                * @return the data read
                * @throws IOException if it reaches end of file before finish reading
                */
                pimpl::Data readData();

                /**
                * @return current position index
                */
                int position();

                /**
                * Move cursor to given index
                * @param newPos new position index to be set
                */
                void position(int newPos);

            private:
                pimpl::DataInput &dataInput;
                pimpl::SerializerHolder &serializerHolder;

                ObjectDataInput(const ObjectDataInput &);

                void operator=(const ObjectDataInput &);

                template<typename T>
                T *getBackwardCompatiblePointer(void *actualData, const T *typePointer) const {
                    return reinterpret_cast<T *>(actualData);
                }

                template<typename T>
                std::unique_ptr<T>
                readObjectInternal(int32_t typeId, const std::shared_ptr<SerializerBase> &serializer) {
                    switch (typeId) {
                        case serialization::pimpl::SerializationConstants::CONSTANT_TYPE_DATA: {
                            serialization::pimpl::DataSerializer *dataSerializer =
                                    static_cast<serialization::pimpl::DataSerializer *>(serializer.get());
                            return dataSerializer->readObject<T>(*this);
                        }
                        case serialization::pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE: {
                            serialization::pimpl::PortableSerializer *portableSerializer =
                                    static_cast<serialization::pimpl::PortableSerializer *>(serializer.get());

                            return portableSerializer->readObject<T>(*this);
                        }
                        default: {
                            std::shared_ptr<StreamSerializer> streamSerializer = std::static_pointer_cast<StreamSerializer>(
                                    serializer);
                            return std::unique_ptr<T>(getBackwardCompatiblePointer<T>(streamSerializer->read(*this),
                                                                                    (T *) NULL));
                        }
                    }
                }
            };

            /**
             * This method is needed for handling backward compatibility with the originally designed api where it
             * assumed that the string in array can not be nullable.
             */
            template<>
            HAZELCAST_API std::vector<std::string> *ObjectDataInput::getBackwardCompatiblePointer(void *actualData,
                                                                                                  const std::vector<std::string> *typePointer) const;


            /**
             * Specialize readObjectInternal method for HazelcastJsonValue to avoid the need for empty
             * HazelcastJsonValue constructor.
             */
            template<>
            std::unique_ptr<HazelcastJsonValue>
            HAZELCAST_API
            ObjectDataInput::readObjectInternal(int32_t typeId, const std::shared_ptr<SerializerBase> &serializer);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_DATA_INPUT */

