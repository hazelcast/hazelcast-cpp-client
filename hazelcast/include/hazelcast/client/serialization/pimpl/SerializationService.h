/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/util/Disposable.h"
#include "hazelcast/client/PartitionAware.h"

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
        class TypedData;

        namespace serialization {
            namespace pimpl {
                /**
                 * This class represents the type of a Hazelcast serializable object. The fields can take the following
                 * values:
                 * 1. Primitive types: factoryId=-1, classId=-1, typeId is the type id for that primitive as listed in
                 * @link SerializationConstants
                 * 2. Array of primitives: factoryId=-1, classId=-1, typeId is the type id for that array as listed in
                 * @link SerializationConstants
                 * 3. IdentifiedDataSerializable: factory, class and type ids are non-negative values as registered by
                 * the DataSerializableFactory.
                 * 4. Portable: factory, class and type ids are non-negative values as registered by the PortableFactory.
                 * 5. Custom serialized objects: factoryId=-1, classId=-1, typeId is the non-negative type id as
                 * registered for the custom object.
                 *
                 */
                struct HAZELCAST_API ObjectType {
                    ObjectType();

                    int32_t typeId;
                    int32_t factoryId;
                    int32_t classId;

                };

                std::ostream HAZELCAST_API &operator<<(std::ostream &os, const ObjectType &type);

                class HAZELCAST_API SerializationService : public util::Disposable {
                public:
                    SerializationService(const SerializationConfig& serializationConfig);

                    /**
                    *
                    *  return false if a serializer is already given corresponding to serializerId
                    */
                    bool registerSerializer(boost::shared_ptr<StreamSerializer> serializer);

                    template<typename T>
                    inline Data toData(const T *object) {
                        DataOutput output;

                        SerializerHolder &serializerHolder = getSerializerHolder();
                        ObjectDataOutput dataOutput(output, &serializerHolder);

                        writeHash<T>(object, output);

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

                        int32_t typeId = data.getType();
                        
                        // Constant 8 is Data::DATA_OFFSET. Windows DLL export does not
                        // let usage of static member.
                        DataInput dataInput(data.toByteArray(), 8);

                        SerializerHolder &serializerHolder = getSerializerHolder();
                        ObjectDataInput objectDataInput(dataInput, serializerHolder);
                        return objectDataInput.readObject<T>(typeId);
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

                    ObjectType getObjectType(const Data *data);

                    /**
                     * This method is public only for testing purposes
                     * @return The serializer holder.
                     */
                    SerializerHolder &getSerializerHolder();

                    /**
                     * @link Disposable interface implementation
                     */
                    void dispose();
                private:

                    SerializationService(const SerializationService &);

                    SerializationService &operator = (const SerializationService &);

                    PortableContext portableContext;
                    const SerializationConfig& serializationConfig;

                    bool isNullData(const Data &data);

                    template<typename T>
                    void writeHash(const PartitionAwareMarker *obj, DataOutput &out) {
                        typedef typename T::KEY_TYPE PK_TYPE;
                        const PartitionAware<PK_TYPE> *partitionAwareObj = static_cast<const PartitionAware<PK_TYPE> *>(obj);
                        const PK_TYPE *pk = partitionAwareObj->getPartitionKey();
                        if (pk != NULL) {
                            Data partitionKey = toData<PK_TYPE>(pk);
                            out.writeInt(partitionKey.getPartitionHash());
                        }
                    }

                    template<typename T>
                    void writeHash(const void *obj, DataOutput &out) {
                        out.writeInt(0);
                    }

                    void registerConstantSerializers();
                };

                template <>
                Data SerializationService::toData(const TypedData *object);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_SERVICE */

