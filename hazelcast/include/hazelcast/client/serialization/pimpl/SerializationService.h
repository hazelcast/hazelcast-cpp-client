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

                    SerializationService(const SerializationConfig& serializationConfig);

                    /**
                    *
                    *  return false if a serializer is already given corresponding to serializerId
                    */
                    bool registerSerializer(boost::shared_ptr<SerializerBase> serializer);

                    template<typename T>
                    inline Data toData(const T *object) {
                        if (NULL == object) {
                            return Data();
                        }

                        DataOutput output;

                        ObjectDataOutput dataOutput(output, portableContext);

                        writeHash(output);

                        dataOutput.writeObject<T>(object);

                        Data data(output.toByteArray());
                        return data;
                    }

                    template<typename T>
                    inline boost::shared_ptr<Data> toSharedData(const T *object) {
                        if (NULL == object) {
                            return boost::shared_ptr<Data>();
                        }
                        return boost::shared_ptr<Data>(new Data(toData<T>(object)));
                    }

                    template<typename T>
                    inline std::auto_ptr<T> toObject(const Data *data) {
                        if (NULL == data) {
                            return std::auto_ptr<T>();
                        }
                        return toObject<T>(*data);
                    }

                    template<typename T>
                    inline std::auto_ptr<T> toObject(const Data &data) {
                        if (isNullData(data)) {
                            return std::auto_ptr<T>();
                        }

                        // Constant 4 is Data::TYPE_OFFSET. Windows DLL export does not
                        // let usage of static member.
                        DataInput dataInput(data.toByteArray(), 4);

                        ObjectDataInput objectDataInput(dataInput, portableContext);
                        return objectDataInput.readObject<T>();
                    }

                    template<typename T>
                    inline const boost::shared_ptr<T> toSharedObject(const boost::shared_ptr<Data> &data) {
                        return boost::shared_ptr<T>(toObject<T>(data.get()));
                    }

                    template<typename T>
                    inline const boost::shared_ptr<T> toSharedObject(const boost::shared_ptr<T> &obj) {
                        return obj;
                    }

                    const byte getVersion() const;

                private:
                    SerializerHolder &getSerializerHolder();

                    SerializationService(const SerializationService &);

                    SerializationService &operator = (const SerializationService &);

                    SerializationConstants constants;
                    PortableContext portableContext;
                    const SerializationConfig& serializationConfig;

                    bool isNullData(const Data &data);

                    void writeHash(DataOutput &out);
                };

                template<>
                HAZELCAST_API Data SerializationService::toData<byte>(const byte  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<bool>(const bool  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<char>(const char  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<int16_t>(const int16_t  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<int32_t>(const int32_t  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<int64_t >(const int64_t  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<float>(const float  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<double>(const double  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<char> >(const std::vector<char> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<bool> >(const std::vector<bool> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<byte> >(const std::vector<byte> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<int16_t> >(const std::vector<int16_t> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<int32_t> >(const std::vector<int32_t> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<int64_t > >(const std::vector<int64_t > *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<float> >(const std::vector<float> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<double> >(const std::vector<double> *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::string>(const std::string  *object);

                template<>
                HAZELCAST_API Data SerializationService::toData<std::vector<std::string> >(const std::vector<std::string> *object);

                template<>
                HAZELCAST_API std::auto_ptr<byte> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<bool> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<char> SerializationService::toObject(const Data &data) ;

                template<>
                HAZELCAST_API std::auto_ptr<int16_t> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<int32_t> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<int64_t > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<float> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<double> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<char> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<bool> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<byte> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<int16_t> >  SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<int32_t> > SerializationService::toObject(const Data &data);
                template<>
                HAZELCAST_API std::auto_ptr<std::vector<int64_t > > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr< std::vector<float> >  SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<double> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::string> SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<std::vector<std::string> > SerializationService::toObject(const Data &data);

                template<>
                HAZELCAST_API std::auto_ptr<Data> SerializationService::toObject(const Data *data);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_SERVICE */

