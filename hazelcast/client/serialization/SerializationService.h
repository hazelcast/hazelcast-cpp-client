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
#include "DataOutput.h"
#include "DataInput.h"
#include "SerializationContext.h"
#include "HazelcastException.h"
#include "StringUtil.h"
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
                    DataOutput *output = pop();
                    int typeID = getTypeId(object);
                    (*output) << object;
                    Data data(typeID, output->toByteArray());
                    push(output);
                    data.cd = serializationContext.lookup(getFactoryId(object), getClassId(object));
                    return data;
                };

                Data toData(Data&);

                template<typename K>
                inline K toObject(const Data& data) {
                    if (data.bufferSize() == 0)
                        throw hazelcast::client::HazelcastException("Empty Data");
                    int typeID = data.type;
                    DataInput dataInput(data.buffer);

                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        serializationContext.registerClassDefinition(data.cd);
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        //TODO add dataSerializer
                    } else {
                        std::string error = "There is no suitable de-serializer for id " + hazelcast::client::util::StringUtil::to_string(typeID);
                        throw hazelcast::client::HazelcastException(error);
                    }
                    K object;
                    dataInput >> object;
                    return object;
                };

                void push(DataOutput *);

                DataOutput *pop();

                static long combineToLong(int x, int y);

                static int extractInt(long value, bool lowerBits);

                SerializationContext *getSerializationContext() {
                    return &serializationContext;
                }

            private:

                SerializationService(const SerializationService&);

                std::queue<DataOutput *> outputPool;

                SerializationContext serializationContext;
            };


        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_SERVICE */
