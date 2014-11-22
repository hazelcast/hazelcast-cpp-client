//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableWriter.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableSerializer::PortableSerializer(PortableContext& portableContext)
                : context(portableContext) {

                }

                void PortableSerializer::write(DataOutput& dataOutput, const Portable& p) {
                    boost::shared_ptr<ClassDefinition> cd = context.lookupOrRegisterClassDefinition(p);


                    util::ByteBuffer& headerBuffer = dataOutput.getHeaderBuffer();
                    int pos = headerBuffer.position();
                    dataOutput.writeInt(pos);

                    headerBuffer.writeInt(cd->getFactoryId());
                    headerBuffer.writeInt(cd->getClassId());
                    headerBuffer.writeInt(cd->getVersion());

                    DefaultPortableWriter dpw(context, cd, dataOutput);
                    PortableWriter portableWriter(&dpw);
                    p.writePortable(portableWriter);
                    portableWriter.end();
                }

                void PortableSerializer::read(DataInput& dataInput, Portable& portable) {

                    util::ByteBuffer& byteBuffer = dataInput.getHeaderBuffer();
                    int headerBufferPos = dataInput.readInt();
                    byteBuffer.position((size_t)headerBufferPos);

                    int factoryId = byteBuffer.readInt();
                    int classId = byteBuffer.readInt();
                    int version = byteBuffer.readInt();

                    int portableVersion = findPortableVersion(factoryId, classId, portable);
                    PortableReader reader = createReader(dataInput, factoryId, classId, version, portableVersion);
                    portable.readPortable(reader);
                    reader.end();
                }

                PortableReader PortableSerializer::createReader(DataInput& input, int factoryId, int classId, int version, int portableVersion) {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    boost::shared_ptr<ClassDefinition> cd = context.lookup(factoryId, classId, effectiveVersion);
                    if (cd == NULL) {
                        std::stringstream s;
                        s << "Could not find class-definition for ";
                        s << "factory-id: " << factoryId << ", class-id: " << classId << ", version: " << effectiveVersion;
                        throw  exception::HazelcastSerializationException("PortableSerializer::createReader", s.str());
                    }

                    if (portableVersion == effectiveVersion) {
                        PortableReader reader(context, input, cd, true);
                        return reader;
                    } else {
                        PortableReader reader(context, input, cd, false);
                        return reader;
                    }
                }

                int PortableSerializer::findPortableVersion(int factoryId, int classId, const Portable& portable) {
                    int currentVersion = context.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion(&portable, context.getVersion());
                        if (currentVersion > 0) {
                            context.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }
            }
        }
    }
}


