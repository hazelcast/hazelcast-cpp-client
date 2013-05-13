////
////  PortableSerializer.cpp
////  Server
////
////  Created by sancar koyunlu on 1/10/13.
////  Copyright (c) 2013 sancar koyunlu. All rights reserved.
////
//#include "PortableSerializer.h"
//#include "PortableWriter.h"
//#include "PortableReader.h"
//#include "SerializationConstants.h"
//#include "PortableFactory.h"
//#include "SerializationService.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace serialization {
//
//            PortableSerializer::PortableSerializer(SerializationService *const serializationService, std::map<int, PortableFactory const * > const &portableFactories)
//            : service(serializationService)
//            , portableFactories(portableFactories) {
//
//            };
//
//            PortableSerializer::~PortableSerializer() {
//                for (std::map< int, PortableFactory const * >::const_iterator it = portableFactories.begin(); it != portableFactories.end(); ++it) {
//                    delete it->second;
//                }
//            };
//
//            boost::shared_ptr<ClassDefinition> PortableSerializer::getClassDefinition(Portable& p) {
//                boost::shared_ptr<ClassDefinition> cd;
//
//                SerializationContext *const context = getSerializationContext();
//                if (context->isClassDefinitionExists(p.getFactoryId(), p.getClassId())) {
//                    cd = context->lookup(p.getFactoryId(), p.getClassId());
//                } else {
//                    cd.reset(new ClassDefinition(p.getFactoryId(), p.getClassId(), context->getVersion()));
//                    PortableWriter classDefinitionWriter(this, cd, NULL, PortableWriter::CLASS_DEFINITION_WRITER);
//                    p.writePortable(classDefinitionWriter);
//                    context->registerClassDefinition(cd);
//                }
//
//                return cd;
//            };
//
//            void PortableSerializer::write(DataOutput &dataOutput, Portable& p) {
//                boost::shared_ptr<ClassDefinition> cd = getClassDefinition(p);
//                PortableWriter writer(this, cd, &dataOutput, PortableWriter::DEFAULT);
//                p.writePortable(writer);
//
//            };
//
//            std::auto_ptr<Portable> PortableSerializer::read(DataInput& dataInput, int factoryId, int dataClassId, int dataVersion) {
//
//                PortableFactory const *portableFactory;
//                if (portableFactories.count(factoryId) != 0) {
//                    portableFactory = portableFactories.at(factoryId);
//                } else {
//                    throw hazelcast::client::HazelcastException("Could not find PortableFactory for factoryId: " + hazelcast::client::util::StringUtil::to_string(factoryId));
//                }
//
//                std::auto_ptr<Portable> p(portableFactory->create(dataClassId));
//                if (p.get() == NULL) {
//                    throw hazelcast::client::HazelcastException("Could not create Portable for class-id: " + hazelcast::client::util::StringUtil::to_string(factoryId));
//                }
//                SerializationContext *const context = getSerializationContext();
//
//                boost::shared_ptr<ClassDefinition> cd;
//                if (context->getVersion() == dataVersion) {
//                    cd = context->lookup(factoryId, dataClassId); // using context.version
//                    PortableReader reader(this, dataInput, cd, PortableReader::DEFAULT);
//                    p->readPortable(reader);
//                } else {
//                    cd = context->lookup(factoryId, dataClassId, dataVersion); // registered during read
//                    PortableReader reader(this, dataInput, cd, PortableReader::MORPHING);
//                    p->readPortable(reader);
//                }
//                return p;
//            };
//
//            SerializationContext *const PortableSerializer::getSerializationContext() {
//                return service->getSerializationContext();
//            };
//
//            vector<int> const PortableSerializer::getFactoryIds() const {
//                vector<int> factoryIds(portableFactories.size());
//                for (std::map< int, PortableFactory const * >::const_iterator it = portableFactories.begin(); it != portableFactories.end(); ++it) {
//                    factoryIds.push_back(it->first);
//                }
//                return factoryIds;
//            };
//        }
//    }
//}
//
