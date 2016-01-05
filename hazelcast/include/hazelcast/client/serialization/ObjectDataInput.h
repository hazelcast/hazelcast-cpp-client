/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/IOUtil.h"
#include <vector>
#include <boost/shared_ptr.hpp>
#include <string>

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
                ObjectDataInput(pimpl::DataInput&, pimpl::PortableContext&);

                /**
                * fills all content to given byteArray
                * @param byteArray to fill the data in
                */
                void readFully(std::vector<byte>& byteArray);

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
                * @return the short read
                * @throws IOException if it reaches end of file before finish reading
                */
                short readShort();

                /**
                * @return the char read
                * @throws IOException if it reaches end of file before finish reading
                */
                char readChar();

                /**
                * @return the int read
                * @throws IOException if it reaches end of file before finish reading
                */
                int readInt();

                /**
                * @return the long read
                * @throws IOException if it reaches end of file before finish reading
                */
                long readLong();

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
                std::auto_ptr<std::string> readUTF();

                /**
                * @return the byte array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<byte> > readByteArray();

                /**
                * @return the boolean array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<bool> > readBooleanArray();

                /**
                * @return the char array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<char> > readCharArray();

                /**
                * @return the int array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<int> > readIntArray();

                /**
                * @return the long array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<long> > readLongArray();

                /**
                * @return the double array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<double> > readDoubleArray();

                /**
                * @return the float array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<float> > readFloatArray();

                /**
                * @return the short array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<short> > readShortArray();

                /**
                * @return the array of strings
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<std::string> > readUTFArray();

                /**
                * Object can be Portable, IdentifiedDataSerializable or custom serializable
                * for custom serialization @see Serializer
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                boost::shared_ptr<T> readObject() {
                    int typeId = readInt();
                    if (pimpl::SerializationConstants::getInstance()->CONSTANT_TYPE_NULL == typeId) {
                        return boost::shared_ptr<T>(static_cast<T *>(NULL));
                    } else {
                        std::auto_ptr<T> result(new T);
                        if (pimpl::SerializationConstants::getInstance()->CONSTANT_TYPE_DATA == typeId) {
                            readDataSerializable(reinterpret_cast<IdentifiedDataSerializable *>(result.get()));
                        } else if (pimpl::SerializationConstants::getInstance()->CONSTANT_TYPE_PORTABLE == typeId) {
                            readPortable(reinterpret_cast<Portable *>(result.get()));
                        } else {
                            readInternal<T>(typeId, result.get());
                        }
                        return boost::shared_ptr<T>(result.release());
                    }
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

                template <typename T>
                void readInternal(int typeId, T * object) {
                    boost::shared_ptr<SerializerBase> serializer = serializerHolder.serializerFor(typeId);
                    if (NULL == serializer.get()) {
                        const std::string message = "No serializer found for serializerId :"+
                                                     util::IOUtil::to_string(typeId) + ", typename :" +
                                                     typeid(T).name();
                        throw exception::HazelcastSerializationException("ObjectDataInput::readInternal", message);
                    }

                    Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());
                    ObjectDataInput objectDataInput(dataInput, portableContext);
                    s->read(objectDataInput, *object);
                }

                void readPortable(Portable *object);

                void readDataSerializable(IdentifiedDataSerializable * object);

                pimpl::DataInput& dataInput;
                pimpl::PortableContext& portableContext;
                pimpl::SerializerHolder& serializerHolder;

                ObjectDataInput(const ObjectDataInput&);

                void operator=(const ObjectDataInput&);

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_DATA_INPUT */

