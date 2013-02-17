//
//  PortableSerializer.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "PortableSerializer.h"
#include "SerializationConstants.h"
#include "DataInput.h"
#include "DataOutput.h"
#include "DefaultPortableWriter.h"
#include "PortableReader.h"
#include "MorphingPortableReader.h"
#include "SerializationContext.h"
#include "ClassDefinitionWriter.h"
#include <cassert>

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableSerializer::PortableSerializer(SerializationContext* context) : context(context) {

            };

            int PortableSerializer::getTypeId() {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            PortableSerializer::~PortableSerializer() {

            };

            ClassDefinition* PortableSerializer::getClassDefinition(Portable& p) {
                int classId = p.getClassId();
                ClassDefinition* cd;

                if (context->isClassDefinitionExists(classId)) {
                    cd = context->lookup(classId);
                } else {
                    cd = new ClassDefinition(classId, context->getVersion());
                    ClassDefinitionWriter classDefinitionWriter(this, cd);
                    p.writePortable(classDefinitionWriter);
                    context->registerClassDefinition(cd);
                }

                return cd;
            };

            int PortableSerializer::getVersion() {
                return context->getVersion();
            };

            void PortableSerializer::write(DataOutput* dataOutput, Portable& p) {

                ClassDefinition* cd = getClassDefinition(p);
                DefaultPortableWriter writer(this, cd, dataOutput);
                p.writePortable(writer);

            };

            std::auto_ptr<Portable> PortableSerializer::read(DataInput& dataInput) {

                int dataClassId = dataInput.getDataClassId();
                int dataVersion = dataInput.getDataVersion();
                std::auto_ptr<Portable> p = context->createPortable(dataClassId);

                ClassDefinition* cd;
                if (context->getVersion() == dataVersion) {
                    cd = context->lookup(dataClassId); // using context.version
                    PortableReader reader(this, dataInput, cd);
                    p->readPortable(reader);
                } else {
                    cd = context->lookup(dataClassId, dataVersion); // registered during read
                    MorphingPortableReader reader(this, dataInput, cd);
                    p->readPortable(reader);
                }
                return p;

            };

        }
    }
}

