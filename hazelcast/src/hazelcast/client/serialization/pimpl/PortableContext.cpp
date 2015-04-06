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
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

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
                PortableContext::PortableContext(int version)
                : contextVersion(version), serializerHolder(*this){
                }


                int PortableContext::getClassVersion(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).getClassVersion(classId);
                }

                void PortableContext::setClassVersion(int factoryId, int classId, int version) {
                    getClassDefinitionContext(factoryId).setClassVersion(classId, version);
                }

                boost::shared_ptr<ClassDefinition> PortableContext::lookupClassDefinition(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                }

                boost::shared_ptr<ClassDefinition> PortableContext::readClassDefinition(DataInput& in, int factoryId, int classId, int version) {
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

                            // TODO: what there's a null inner Portable field
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

                                // TODO: what there's a null inner Portable field
                                int fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            } else {
                                shouldRegister = false;
                            }

                        }
                        FieldDefinition fieldDef(i, name, type, fieldFactoryId, fieldClassId);
                        builder.addField(fieldDef);
                    }
                    boost::shared_ptr<ClassDefinition> classDefinition = builder.build();
                    if (shouldRegister) {
                        classDefinition = registerClassDefinition(classDefinition);
                    }
                    return classDefinition;
                }

                boost::shared_ptr<ClassDefinition> PortableContext::registerClassDefinition(boost::shared_ptr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                }

                boost::shared_ptr<ClassDefinition> PortableContext::lookupOrRegisterClassDefinition(const Portable& portable) {
                    int portableVersion = PortableVersionHelper::getVersion(&portable, contextVersion);
                    boost::shared_ptr<ClassDefinition> cd = lookupClassDefinition(portable.getFactoryId(), portable.getClassId(), portableVersion);
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

                ClassDefinitionContext& PortableContext::getClassDefinitionContext(int factoryId) {
                    boost::shared_ptr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value == NULL) {
                        value = boost::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(this));
                        boost::shared_ptr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId, value);
                        if (current != NULL) {
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

