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
//  PortableContext.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionContext.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"

using namespace hazelcast::util;
using namespace hazelcast::client::serialization;
using namespace std;

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableContext::PortableContext(int version, const SerializationConstants& constants)
                : contextVersion(version), serializerHolder(*this) , constants(constants){
                }


                int PortableContext::getClassVersion(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).getClassVersion(classId);
                }

                void PortableContext::setClassVersion(int factoryId, int classId, int version) {
                    getClassDefinitionContext(factoryId).setClassVersion(classId, version);
                }

                hazelcast::util::SharedPtr<ClassDefinition> PortableContext::lookupClassDefinition(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                }

                hazelcast::util::SharedPtr<ClassDefinition> PortableContext::readClassDefinition(DataInput& in, int factoryId, int classId, int version) {
                    bool shouldRegister = true;
                    ClassDefinitionBuilder builder(factoryId, classId, version);

                    // final position after portable is read
                    in.readInt();

                    // field count
                    int fieldCount = in.readInt();
                    int offset = in.position();
                    for (int i = 0; i < fieldCount; i++) {
                        in.position(offset + i * Bits::INT_SIZE_IN_BYTES);
                        int pos = in.readInt();
                        in.position(pos);

                        short len = in.readShort();
                        vector<char> chars(len);
                        in.readFully(chars);
                        chars.push_back('\0');

                        FieldType type(in.readByte());
                        std::string name(&(chars[0]));
                        int fieldFactoryId = 0;
                        int fieldClassId = 0;
                        if (type == FieldTypes::TYPE_PORTABLE) {
                            // is null
                            if (in.readBoolean()) {
                                shouldRegister = false;
                            }
                            fieldFactoryId = in.readInt();
                            fieldClassId = in.readInt();

                            // TODO: what if there's a null inner Portable field
                            if (shouldRegister) {
                                int fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            }
                        } else if (type == FieldTypes::TYPE_PORTABLE_ARRAY) {
                            int k = in.readInt();
                            if (k > 0) {
                                fieldFactoryId = in.readInt();
                                fieldClassId = in.readInt();

                                int p = in.readInt();
                                in.position(p);

                                // TODO: what if there's a null inner Portable field
                                int fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            } else {
                                shouldRegister = false;
                            }

                        }
                        FieldDefinition fieldDef(i, name, type, fieldFactoryId, fieldClassId);
                        builder.addField(fieldDef);
                    }
                    hazelcast::util::SharedPtr<ClassDefinition> classDefinition = builder.build();
                    if (shouldRegister) {
                        classDefinition = registerClassDefinition(classDefinition);
                    }
                    return classDefinition;
                }

                hazelcast::util::SharedPtr<ClassDefinition> PortableContext::registerClassDefinition(hazelcast::util::SharedPtr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                }

                hazelcast::util::SharedPtr<ClassDefinition> PortableContext::lookupOrRegisterClassDefinition(const Portable& portable) {
                    int portableVersion = PortableVersionHelper::getVersion(&portable, contextVersion);
                    hazelcast::util::SharedPtr<ClassDefinition> cd = lookupClassDefinition(portable.getFactoryId(), portable.getClassId(), portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(portable.getFactoryId(), portable.getClassId(), portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        portable.writePortable(portableWriter);
                        cd = cdw.registerAndGet();
                    }
                    return cd;
                }

                int PortableContext::getVersion() {
                    return contextVersion;
                }

                SerializerHolder& PortableContext::getSerializerHolder() {
                    return serializerHolder;
                }


                SerializationConstants const& PortableContext::getConstants() const {
                    return constants;
                }

                ClassDefinitionContext& PortableContext::getClassDefinitionContext(int factoryId) {
                    hazelcast::util::SharedPtr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value.get() == NULL) {
                        value = hazelcast::util::SharedPtr<ClassDefinitionContext>(new ClassDefinitionContext(this));
                        hazelcast::util::SharedPtr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId, value);
                        if (current.get() != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                }

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

