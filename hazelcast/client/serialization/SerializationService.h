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
#include "HazelcastException.h"
#include "DataSerializable.h"
#include "Util.h"
#include "Data.h"
#include "boost/type_traits/is_base_of.hpp"
#include "boost/any.hpp"
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
                Data toData(K& object) {
                    BufferedDataOutput output;
                    int typeID;
                    if (boost::is_base_of<Portable, K>::value) {
                        typeID = SerializationConstants::CONSTANT_TYPE_PORTABLE;
                    } else if (boost::is_base_of<DataSerializable, K>::value) {
                        typeID = SerializationConstants::CONSTANT_TYPE_DATA;
                    } else {
                        typeID = getTypeId(object);
                    }
                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        portableSerializer.write(output, object);
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        dataSerializer.write(output, object);
                    } else {
                        writePortable(output, object);
                    }
                    Data data(typeID, output.toByteArray());

                    if (boost::is_base_of<Portable, K>::value) {
                        int factoryId = getFactoryId(object);
                        int classId = getClassId(object);
                        data.cd = serializationContext.lookup(factoryId, classId);
                    }
                    return data;
                };

                Data toData(Data&);

                template<typename K>
                inline K toObject(const Data& data) {
                    if (data.bufferSize() == 0)
                        throw hazelcast::client::HazelcastException("Empty Data");
                    int typeID = data.type;
                    BufferedDataInput dataInput(data.buffer);

                    K object;
                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        serializationContext.registerClassDefinition(data.cd);
                        portableSerializer.read(dataInput, object, data.cd->getFactoryId(), data.cd->getClassId(), data.cd->getVersion());
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        dataSerializer.read(dataInput, object);
                    } else {
                        readPortable(dataInput, object);
                    }
                    return object;
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
