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
//
// Created by İhsan Demir on 25/03/15.
//


#ifndef HAZELCAST_PortableReaderBase_H_
#define HAZELCAST_PortableReaderBase_H_

#include "hazelcast/client/serialization/ObjectDataInput.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ClassDefinition;

            namespace pimpl {

                class DataInput;

				class HAZELCAST_API PortableReaderBase {

                public:
                    PortableReaderBase(PortableContext &portableContext,
                                       ObjectDataInput &input,
                            std::shared_ptr<ClassDefinition> cd);

                    virtual ~PortableReaderBase();

                    virtual int32_t readInt(const char *fieldName);

                    virtual int64_t readLong(const char *fieldName);

                    virtual bool readBoolean(const char *fieldName);

                    virtual byte readByte(const char *fieldName);

                    virtual char readChar(const char *fieldName);

                    virtual double readDouble(const char *fieldName);

                    virtual float readFloat(const char *fieldName);

                    virtual int16_t readShort(const char *fieldName);

                    virtual std::unique_ptr<std::string> readUTF(const char *fieldName);

                    virtual std::unique_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<bool> > readBooleanArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<char> > readCharArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<std::string> > readUTFArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                    ObjectDataInput &getRawDataInput();

                    void end();

                protected:
                    void setPosition(char const * , FieldType const& fieldType);

                    void checkFactoryAndClass(FieldDefinition fd, int factoryId, int classId) const;

                    int readPosition(const char *, FieldType const& fieldType);

                    template <typename T>
                    std::shared_ptr<T> getPortableInstance(char const *fieldName) {
                        setPosition(fieldName, FieldTypes::TYPE_PORTABLE);

                        bool isNull = dataInput.readBoolean();
                        int32_t factoryId = dataInput.readInt();
                        int32_t classId = dataInput.readInt();

                        checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                        if (isNull) {
                            return std::shared_ptr<T>();
                        } else {
                            return read<T>(dataInput, factoryId, classId);
                        }
                    }

                    template <typename T>
                    std::shared_ptr<T> read(ObjectDataInput &dataInput, int32_t factoryId, int32_t classId) const {
                        std::shared_ptr<PortableSerializer> serializer = std::static_pointer_cast<PortableSerializer>(
                                serializerHolder.serializerFor(SerializationConstants::CONSTANT_TYPE_PORTABLE));

                        return std::shared_ptr<T>(serializer->read<T>(dataInput, factoryId, classId));
                    }

                    std::shared_ptr<ClassDefinition> cd;
                    ObjectDataInput &dataInput;
                private:
                    SerializerHolder &serializerHolder;
                    int finalPosition;
                    int offset;
                    bool raw;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PortableReaderBase_H_
