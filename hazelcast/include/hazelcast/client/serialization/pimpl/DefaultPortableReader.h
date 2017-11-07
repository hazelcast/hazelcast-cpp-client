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
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_READER
#define HAZELCAST_PORTABLE_READER

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/PortableReaderBase.h"

#include <string>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif


namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;

            namespace pimpl {
                class PortableContext;

                class HAZELCAST_API DefaultPortableReader : public PortableReaderBase {
                public:

                    DefaultPortableReader(PortableContext &portableContext, ObjectDataInput &input, boost::shared_ptr<ClassDefinition> cd);

                    template<typename T>
                    boost::shared_ptr<T> readPortable(const char *fieldName) {
                        return boost::shared_ptr<T>(static_cast<T *>(getPortableInstance(fieldName,
                                                                                        std::auto_ptr<Portable>(
                                                                                                new T)).release()));
                    };

                    template<typename T>
                    std::vector<T> readPortableArray(const char *fieldName) {
                        PortableReaderBase::setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                        dataInput.readInt();
                        std::vector<T> portables;

                        setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                        int32_t len = dataInput.readInt();
                        int32_t factoryId = dataInput.readInt();
                        int32_t classId = dataInput.readInt();

                        checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                        if (len > 0) {
                            int offset = dataInput.position();
                            for (int i = 0; i < len; i++) {
                                dataInput.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                                int32_t start = dataInput.readInt();
                                dataInput.position(start);

                                std::auto_ptr<Portable> instance = read(dataInput, std::auto_ptr<Portable>(new T),
                                                                        factoryId, classId);
                                portables.push_back(*(static_cast<T *>(instance.get())));
                            }
                        }

                        return portables;
                    }

                };
            }

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_PORTABLE_READER */

