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
#include "PortableWriter.h"
#include "PortableReader.h"
#include "SerializationContext.h"
#include <boost/shared_ptr.hpp>
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

            boost::shared_ptr<ClassDefinition> PortableSerializer::getClassDefinition(Portable& p) {
                int classId = p.getClassId();
                boost::shared_ptr<ClassDefinition> cd;

                if (context->isClassDefinitionExists(classId)) {
                    cd = context->lookup(classId);
                } else {
                    cd.reset(new ClassDefinition(classId, context->getVersion()));
                    PortableWriter classDefinitionWriter(this, cd, NULL, PortableWriter::CLASS_DEFINITION_WRITER);
                    p.writePortable(classDefinitionWriter);
                    context->registerClassDefinition(cd);
                }

                return cd;
            };

            int PortableSerializer::getVersion() {
                return context->getVersion();
            };

            void PortableSerializer::write(DataOutput* dataOutput, Portable& p) {

                boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
                PortableWriter writer(this, cd, dataOutput, PortableWriter::DEFAULT);
                p.writePortable(writer);

            };

            std::auto_ptr<Portable> PortableSerializer::read(DataInput& dataInput) {

                int dataClassId = dataInput.getDataClassId();
                int dataVersion = dataInput.getDataVersion();
                std::auto_ptr<Portable> p = context->createPortable(dataClassId);

                boost::shared_ptr<ClassDefinition> cd;
                if (context->getVersion() == dataVersion) {
                    cd = context->lookup(dataClassId); // using context.version
                    PortableReader reader(this, dataInput, cd, PortableReader::DEFAULT);
                    p->readPortable(reader);
                } else {
                    cd = context->lookup(dataClassId, dataVersion); // registered during read
                    PortableReader reader(this, dataInput, cd, PortableReader::MORPHING);
                    p->readPortable(reader);
                }
                return p;

            };

        }
    }
}

