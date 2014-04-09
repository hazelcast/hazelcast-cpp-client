//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/pimpl/SerializationContext.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableSerializer::PortableSerializer(SerializationContext &serializationContext)
                : context(serializationContext) {

                };

                boost::shared_ptr<ClassDefinition> PortableSerializer::getClassDefinition(const Portable &p) {
                    boost::shared_ptr<ClassDefinition> cd;
                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();
                    if (context.isClassDefinitionExists(factoryId, classId)) {
                        cd = context.lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter cdw(factoryId, classId, context.getVersion(), context);
                        PortableWriter classDefinitionWriter(&cdw);
                        p.writePortable(classDefinitionWriter);
                        cd = cdw.getClassDefinition();
                        cd = context.registerClassDefinition(cd);
                    }

                    return cd;
                };

                void PortableSerializer::write(DataOutput &dataOutput, const Portable &p) {
                    boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
                    DefaultPortableWriter dpw(context, cd, dataOutput);
                    PortableWriter portableWriter(&dpw);
                    p.writePortable(portableWriter);
                    portableWriter.end();
                };

                void PortableSerializer::read(DataInput &dataInput, Portable &object, int factoryId, int classId, int dataVersion) {
                    boost::shared_ptr<ClassDefinition> cd;

                    if (context.getVersion() == dataVersion) {
                        cd = context.lookup(factoryId, classId); // using serializationContext.version
                        if (cd == NULL) {
                            throw exception::IOException("PortableSerializer::read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion)",
                                    "Could not find class-definition for factory-id: " + util::IOUtil::to_string(factoryId) + ", class-id: " + util::IOUtil::to_string(classId) + ", version: " + util::IOUtil::to_string(dataVersion));
                        }
                        DefaultPortableReader defaultPortableReader(context, dataInput, cd);
                        PortableReader reader(&defaultPortableReader);
                        object.readPortable(reader);
                        reader.end();
                    } else {
                        cd = context.lookup(factoryId, classId, dataVersion); // registered during read
                        if (cd == NULL) {
                            throw exception::IOException("PortableSerializer::read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion)",
                                    "Could not find class-definition for factory-id: " + util::IOUtil::to_string(factoryId) + ", class-id: " + util::IOUtil::to_string(classId) + ", version: " + util::IOUtil::to_string(dataVersion));
                        }
                        MorphingPortableReader morphingPortableReader(context, dataInput, cd);
                        PortableReader reader(&morphingPortableReader);
                        object.readPortable(reader);
                        reader.end();
                    }
                };
            }
        }
    }
}


