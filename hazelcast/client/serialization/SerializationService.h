//
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_SERVICE
#define HAZELCAST_SERIALIZATION_SERVICE

#include "ConstantSerializers.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "PortableSerializer.h"
#include "SerializationContext.h"
#include "../HazelcastException.h"
#include "DataSerializable.h"
#include "../util/Util.h"
#include "Data.h"
#include "DataSerializer.h"
#include <iosfwd>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {


            class SerializationService {
            public:

                SerializationService(int version);

                ~SerializationService();

                template<typename K>
                void toData(K& object, Data& data) {
                    BufferedDataOutput output;
                    int typeID = getTypeId(object);
                    data.setType(typeID);
                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        portableSerializer.write(output, object);
                        int factoryId = getFactoryId(object);
                        int classId = getClassId(object);
                        data.cd = serializationContext.lookup(factoryId, classId);
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        dataSerializer.write(output, object);
                    } else {
                        writePortable(output, object);
                    }
                    data.setBuffer(output.toByteArray());
                };

                template<typename K>
                inline void toObject(const Data& data, K & object) {
                    if (data.bufferSize() == 0)
                        return;
                    int typeID = data.type;
                    BufferedDataInput dataInput(*(data.buffer.get()));

                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        serializationContext.registerClassDefinition(data.cd);
                        int factoryId = data.cd->getFactoryId();
                        int classId = data.cd->getClassId();
                        int version = data.cd->getVersion();
                        portableSerializer.read(dataInput, object, factoryId, classId, version);
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        dataSerializer.read(dataInput, object);
                    } else {
                        readPortable(dataInput, object);
                    }
                };

                SerializationContext *getSerializationContext() {
                    return &serializationContext;
                }

            private:

                SerializationService(const SerializationService&);

                SerializationContext serializationContext;
                PortableSerializer portableSerializer;
                DataSerializer dataSerializer;
            };
        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_SERVICE */
