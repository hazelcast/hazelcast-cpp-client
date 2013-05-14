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
#include "Data.h"
#include "PortableSerializer.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include "SerializationContext.h"
#include "HazelcastException.h"
#include "Util.h"
#include "boost/type_traits/is_base_of.hpp"
#include "boost/any.hpp"
#include <iostream>
#include <string>
#include <map>
#include <queue>

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
                    int typeID; //TODO uncomment following lines
//                    if(boost::is_base_of<Portable, K>::value){
//                        typeID = SerializationConstants::CONSTANT_TYPE_PORTABLE;
//                    }else if(boost::is_base_of<DataSerializable, K>::value){
//                        typeID = SerializationConstants::CONSTANT_TYPE_DATA;
//                    } else {
                    typeID = getTypeId(object);
//                    }
//                    if(typeID == 0){
//                        throw hazelcast::client::HazelcastException("given class must be either one of primitives,  vector of primitives , DataSeriliazable or Portable)");
//                    }
                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        PortableSerializer portableSerializer(this);//TODO is service necessary as argument // context can be sufficient
                        portableSerializer.write(output, object);
                    } else {
                        output << object;
                    }
                    Data data(typeID, output.toByteArray());
                    int factoryId = getFactoryId(object);
                    int classId = getClassId(object);

                    if (serializationContext.isClassDefinitionExists(factoryId, classId)) {
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

                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        serializationContext.registerClassDefinition(data.cd);
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        //TODO add dataSerializer NOT SURE
                    } else {
                        std::string error = "There is no suitable de-serializer for id " + hazelcast::client::util::to_string(typeID);
                        throw hazelcast::client::HazelcastException(error);
                    }
                    K object;
                    dataInput >> object;
                    return object;
                };

                static long combineToLong(int x, int y);

                static int extractInt(long value, bool lowerBits);

                SerializationContext *getSerializationContext() {
                    return &serializationContext;
                }

            private:

                SerializationService(const SerializationService&);

                SerializationContext serializationContext;
            };


        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_SERVICE */
