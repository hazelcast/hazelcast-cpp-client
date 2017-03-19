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
//  MorphingPortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_MORPHING_PORTABLE_READER
#define HAZELCAST_MORPHING_PORTABLE_READER

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/pimpl/PortableReaderBase.h"

#include <string>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;

            namespace pimpl {

                class SerializerHolder;

                class HAZELCAST_API MorphingPortableReader : public PortableReaderBase {
                public:

                    MorphingPortableReader(PortableContext &portableContext, DataInput &input, boost::shared_ptr<ClassDefinition> cd);

                    int32_t readInt(const char *fieldName);

                    int64_t readLong(const char *fieldName);

                    bool readBoolean(const char *fieldName);

                    byte readByte(const char *fieldName);

                    char readChar(const char *fieldName);

                    double readDouble(const char *fieldName);

                    float readFloat(const char *fieldName);

                    int16_t readShort(const char *fieldName);

                    std::auto_ptr<std::string> readUTF(const char *fieldName);

                    std::auto_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                    std::auto_ptr<std::vector<char> > readCharArray(const char *fieldName);

                    std::auto_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                    std::auto_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                    std::auto_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                    std::auto_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                    std::auto_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                    template<typename T>
                    boost::shared_ptr<T> readPortable(const char *fieldName) {
                        setPosition(fieldName, FieldTypes::TYPE_PORTABLE);
                        boost::shared_ptr<T> portableInstance(new T);

                        Portable * p = portableInstance.get();
                        getPortableInstance(fieldName, p);
                        return portableInstance;
                    };

                    template<typename T>
                    std::vector<T> readPortableArray(const char *fieldName) {
                        PortableReaderBase::setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                        int32_t len = dataInput.readInt();
                        std::vector<T> portables(len);

                        std::vector<Portable *> baseArray(len);
                        int i = 0;
                        for (typename std::vector<T>::iterator it = portables.begin();
                             portables.end() != it; ++it) {
                            baseArray[i++] = (Portable *)(&(*it));
                        }

                        getPortableInstancesArray(fieldName, baseArray);
                        return portables;
                    };
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_MORPHING_PORTABLE_READER */


