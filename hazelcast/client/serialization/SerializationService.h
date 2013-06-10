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
#include "../../util/Util.h"
#include "Data.h"
#include "DataSerializer.h"
#include "TypeSerializer.h"
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
                    Data data;
                    BufferedDataOutput output;
                    int typeID = getTypeSerializerId(object);
                    data.setType(typeID);
                    if (typeID == SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                        portableSerializer.write(output, object);
                        int factoryId = getFactoryId(object);
                        int classId = getClassId(object);
                        data.cd = serializationContext.lookup(factoryId, classId);
                    } else if (typeID == SerializationConstants::CONSTANT_TYPE_DATA) {
                        dataSerializer.write(output, object);
                    } else {
                        throw HazelcastException("Not supported");
                    }
                    data.setBuffer(output.toByteArray());
                    return data;
                };

                template<typename K>
                inline K toObject(const Data& data) {
                    K object;
                    if (data.bufferSize() == 0)
                        return object;
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
                        throw HazelcastException("Not supported");
                    }
                    return object;
                };

                SerializationContext& getSerializationContext();

                Data toData(byte);

                Data toData(bool);

                Data toData(char);

                Data toData(short);

                Data toData(int);

                Data toData(long);

                Data toData(float);

                Data toData(double);

                Data toData(const std::vector<byte>&);

                Data toData(const std::vector<char>&);

                Data toData(const std::vector<short>&);

                Data toData(const std::vector<int>&);

                Data toData(const std::vector<long>&);

                Data toData(const std::vector<float>&);

                Data toData(const std::vector<double>&);

                Data toData(const std::string&);

            private:

                SerializationService(const SerializationService&);

                SerializationContext serializationContext;
                PortableSerializer portableSerializer;
                DataSerializer dataSerializer;

            };

            template<>
            inline byte SerializationService::toObject(const Data& data) {
                byte object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline bool SerializationService::toObject(const Data& data) {
                bool object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline char SerializationService::toObject(const Data& data) {
                char object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline short SerializationService::toObject(const Data& data) {
                short object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline int SerializationService::toObject(const Data& data) {
                int object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline long SerializationService::toObject(const Data& data) {
                long object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline float SerializationService::toObject(const Data& data) {
                float object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline double SerializationService::toObject(const Data& data) {
                double object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<byte> SerializationService::toObject(const Data& data) {
                std::vector<byte> object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<char> SerializationService::toObject(const Data& data) {
                std::vector<char> object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<short> SerializationService::toObject(const Data& data) {
                std::vector<short > object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<int> SerializationService::toObject(const Data& data) {
                std::vector<int> object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<long> SerializationService::toObject(const Data& data) {
                std::vector<long> object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<float> SerializationService::toObject(const Data& data) {
                std::vector<float> object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::vector<double> SerializationService::toObject(const Data& data) {
                std::vector<double > object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };

            template<>
            inline std::string SerializationService::toObject(const Data& data) {
                std::string object;
                BufferedDataInput dataInput(*(data.buffer.get()));
                readPortable(dataInput, object);
                return object;
            };
        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_SERVICE */
