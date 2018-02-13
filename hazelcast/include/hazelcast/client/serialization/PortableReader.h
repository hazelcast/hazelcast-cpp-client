/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 8/10/13.





#ifndef HAZELCAST_PortableReader
#define HAZELCAST_PortableReader

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/serialization/pimpl/MorphingPortableReader.h"
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                class PortableContext;

                class DataInput;
            }

            class ClassDefinition;

            /**
            * Provides a mean of reading portable fields from a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableReader {
            public:

                /**
                * Internal Api constructor
                */
                PortableReader(pimpl::PortableContext& context, ObjectDataInput& dataInput,
                                boost::shared_ptr<ClassDefinition> cd, bool isDefaultReader);

                PortableReader(const PortableReader & reader);

                PortableReader &operator = (const PortableReader & reader);

                /**
                * @param fieldName name of the field
                * @return the int32_t value read
                * @throws IOException
                */
                int32_t readInt(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int64_t value read
                * @throws IOException
                */
                int64_t readLong(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the boolean value read
                * @throws IOException
                */
                bool readBoolean(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the byte value read
                * @throws IOException
                */
                byte readByte(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the char value read
                * @throws IOException
                */
                char readChar(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the double value read
                * @throws IOException
                */
                double readDouble(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the float value read
                * @throws IOException
                */
                float readFloat(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int32_t value read
                * @throws IOException
                */
                int16_t readShort(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the utf string value read
                * @throws IOException
                */
                std::auto_ptr<std::string> readUTF(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the byte array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the bool array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<bool> > readBooleanArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the char array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<char> > readCharArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int32_t array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int64_t array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the double array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the float array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int16_t array value read
                * @throws IOException
                */
                std::auto_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @return the portable value read
                * @throws IOException
                */
                template<typename T>
                boost::shared_ptr<T> readPortable(const char *fieldName) {
                    if (isDefaultReader)
                        return defaultPortableReader->readPortable<T>(fieldName);
                    return morphingPortableReader->readPortable<T>(fieldName);
                };

                /**
                * @tparam type of the portable class in array
                * @param fieldName name of the field
                * @return the portable array value read
                * @throws IOException
                */
                template<typename T>
                std::vector<T> readPortableArray(const char *fieldName) {
                    if (isDefaultReader)
                        return defaultPortableReader->readPortableArray<T>(fieldName);
                    return morphingPortableReader->readPortableArray<T>(fieldName);
                };

                /**
                * @see PortableWriter#getRawDataOutput
                *
                * Note that portable fields can not read after getRawDataInput() is called. In case this happens,
                * IOException will be thrown.
                *
                * @return rawDataInput
                * @throws IOException
                */
                ObjectDataInput& getRawDataInput();

                /**
                * Internal Api. Should not be called by end user.
                */
                void end();

            private:
                bool isDefaultReader;
                mutable std::auto_ptr<pimpl::DefaultPortableReader> defaultPortableReader;
                mutable std::auto_ptr<pimpl::MorphingPortableReader> morphingPortableReader;

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PortableReader

