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
#include "hazelcast/client/serialization/PortableFactory.h"
#include "hazelcast/client/SerializationConfig.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                PortableSerializer::PortableSerializer(PortableContext& portableContext)
                : context(portableContext) {
                }

                void PortableSerializer::write(ObjectDataOutput& out, const Portable& p) {
                    boost::shared_ptr<ClassDefinition> cd = context.lookupOrRegisterClassDefinition(p);
                    out.writeInt(cd->getVersion());

                    DefaultPortableWriter dpw(context, cd, out);
                    PortableWriter portableWriter(&dpw);
                    p.writePortable(portableWriter);
                    portableWriter.end();
                }

                void PortableSerializer::read(ObjectDataInput &in, Portable &p) {
                    int version = in.readInt();

                    int factoryId = p.getFactoryId();
                    int classId = p.getClassId();

                    int portableVersion = findPortableVersion(factoryId, classId, p);

                    PortableReader reader = createReader(in, factoryId, classId, version, portableVersion);
                    p.readPortable(reader);
                    reader.end();
                }

                void *PortableSerializer::create(ObjectDataInput &in) {
                    int32_t factoryId = in.readInt();
                    int32_t classId = in.readInt();

                    return createNewPortableInstance(factoryId, classId).release();
                }

                PortableReader PortableSerializer::createReader(ObjectDataInput& input, int factoryId, int classId, int version, int portableVersion) const {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    boost::shared_ptr<ClassDefinition> cd = context.lookupClassDefinition(factoryId, classId, effectiveVersion);
                    if (cd == NULL) {
                        int begin = input.position();
                        cd = context.readClassDefinition(input, factoryId, classId, effectiveVersion);
                        input.position(begin);
                    }

                    if (portableVersion == effectiveVersion) {
                        PortableReader reader(context, input, cd, true);
                        return reader;
                    } else {
                        PortableReader reader(context, input, cd, false);
                        return reader;
                    }
                }

                int PortableSerializer::findPortableVersion(int factoryId, int classId, const Portable& portable) const {
                    int currentVersion = context.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion(&portable, context.getVersion());
                        if (currentVersion > 0) {
                            context.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                std::auto_ptr<Portable>
                PortableSerializer::createNewPortableInstance(int32_t factoryId, int32_t classId) {
                    const std::map<int32_t, boost::shared_ptr<PortableFactory> > &portableFactories =
                            context.getSerializationConfig().getPortableFactories();
                    std::map<int, boost::shared_ptr<hazelcast::client::serialization::PortableFactory> >::const_iterator factoryIt =
                            portableFactories.find(factoryId);
                    
                    if (portableFactories.end() == factoryIt) {
                        return std::auto_ptr<Portable>();
                    }

                    return factoryIt->second->create(classId);
                }

                int32_t PortableSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_PORTABLE;
                }

            }
        }
    }
}


