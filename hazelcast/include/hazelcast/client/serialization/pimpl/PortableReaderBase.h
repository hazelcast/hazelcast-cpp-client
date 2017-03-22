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
                            DataInput &input,
                            boost::shared_ptr<ClassDefinition> cd);

                    virtual ~PortableReaderBase();

                    virtual int32_t readInt(const char *fieldName);

                    virtual int64_t readLong(const char *fieldName);

                    virtual bool readBoolean(const char *fieldName);

                    virtual byte readByte(const char *fieldName);

                    virtual char readChar(const char *fieldName);

                    virtual double readDouble(const char *fieldName);

                    virtual float readFloat(const char *fieldName);

                    virtual int16_t readShort(const char *fieldName);

                    virtual std::auto_ptr<std::string> readUTF(const char *fieldName);

                    virtual std::auto_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<bool> > readBooleanArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<char> > readCharArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                    virtual std::auto_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                    ObjectDataInput &getRawDataInput();

                    void end();

                protected:
                    void getPortableInstance(char const *fieldName,
                            Portable * &portableInstance);

                    void getPortableInstancesArray(char const *fieldName,
                            std::vector<Portable *> &portableInstances);

                    void setPosition(char const * , FieldType const& fieldType);

                    boost::shared_ptr<ClassDefinition> cd;
                    DataInput &dataInput;
                private:
                    SerializerHolder &serializerHolder;
                    int finalPosition;
                    ObjectDataInput objectDataInput;
                    int offset;
                    bool raw;

                    void checkFactoryAndClass(FieldDefinition fd, int factoryId, int classId) const;

                    void read(DataInput &dataInput, Portable &object, int factoryId, int classId) const;

                    int readPosition(const char *, FieldType const& fieldType);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PortableReaderBase_H_
