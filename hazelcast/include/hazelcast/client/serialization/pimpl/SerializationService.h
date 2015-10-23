/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//  SerializationService.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_SERVICE
#define HAZELCAST_SERIALIZATION_SERVICE

#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/PortableSerializer.h"
#include "hazelcast/client/serialization/pimpl/DataSerializer.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/common/containers/ManagedPointerVector.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ByteBuffer.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <list>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class SerializationConfig;

        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API SerializationService {
                public:
                    #define CHECK_NULL(type) \
                    if (isNullData(data)) { \
                        return boost::shared_ptr<type >(); \
                    }\

                    SerializationService(const SerializationConfig& serializationConfig);

                    /**
                    *
                    *  return false if a serializer is already given corresponding to serializerId
                    */
                    bool registerSerializer(boost::shared_ptr<SerializerBase> serializer);

                    template<typename T>
                    inline Data toData(const T *object) {
                        DataOutput output;

                        ObjectDataOutput dataOutput(output, portableContext);

                        writeObject<T>(dataOutput, object);

                        writeHash(output);

                        Data data(output.toByteArray());
                        return data;
                    }

                    template<typename T>
                    inline boost::shared_ptr<T> toObject(const Data &data) {
                        CHECK_NULL(T);

                        DataInput dataInput(data.toByteArray());

                        return readObject<T>(dataInput);
                    }

                    PortableContext &getPortableContext();

                private:
                    template <typename T>
                    inline boost::shared_ptr<T> readObject(DataInput &data) {
                        ObjectDataInput dataInput(data, portableContext);
                        return dataInput.readObject<T>();
                    }

                    template <typename T>
                    inline void writeObject(ObjectDataOutput &dataOutput, const T *object) {
                        dataOutput.writeObject<T>(object);
                    }

                    SerializerHolder &getSerializerHolder();

                    boost::shared_ptr<SerializerBase> serializerFor(int typeId);

                    SerializationService(const SerializationService &);

                    SerializationService &operator = (const SerializationService &);

                    PortableContext portableContext;

                    const SerializationConfig& serializationConfig;

                    void checkClassType(int expectedType, int currentType);

                    static bool isNullData(const Data &data);

                    void writeHash(DataOutput &out);
                };

                template<>
                HAZELCAST_API Data SerializationService::toData<byte>(const byte  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<bool>(const bool  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<char>(const char  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<short>(const short  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<int>(const int  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<long>(const long *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<float>(const float  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<double>(const double  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<char> >(const std::vector<char> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<bool> >(const std::vector<bool> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<short> >(const std::vector<short> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<int> >(const std::vector<int> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<long> >(const std::vector<long> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<float> >(const std::vector<float> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<double> >(const std::vector<double> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::string>(const std::string  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<const std::string *> >(const std::vector<const std::string *> *object);

                template<>
                HAZELCAST_API boost::shared_ptr<byte> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<bool> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<char> SerializationService::toObject(const Data &data) ;

                template<>
                HAZELCAST_API boost::shared_ptr<short> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<int> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<long> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<float> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<double> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<std::vector<char> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<std::vector<bool> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<std::vector<short> >  SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<std::vector<int> > SerializationService::toObject(const Data &data);
                template<>
                HAZELCAST_API boost::shared_ptr<std::vector<long> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr< std::vector<float> >  SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<std::vector<double> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<std::string> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API boost::shared_ptr<common::containers::ManagedPointerVector<std::string> > SerializationService::toObject(const Data &data);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_SERVICE */

