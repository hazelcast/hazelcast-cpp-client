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

#ifndef HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_
#define HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/Serializer.h"

namespace hazelcast {
    namespace client {

        class SerializationConfig;

        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            namespace pimpl {
                class HAZELCAST_API NullSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ByteSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API BooleanSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API CharSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ShortSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API IntegerSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API LongSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API FloatSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API DoubleSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API StringSerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API TheByteArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API BooleanArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API CharArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ShortArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API IntegerArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API LongArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API FloatArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API DoubleArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API StringArraySerializer : public SerializerBase {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_

