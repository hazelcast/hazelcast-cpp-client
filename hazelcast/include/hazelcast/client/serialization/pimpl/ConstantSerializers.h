/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once
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
                class HAZELCAST_API NullSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ByteSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API BooleanSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API CharSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ShortSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API IntegerSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API LongSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API FloatSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API DoubleSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API StringSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API TheByteArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API BooleanArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API CharArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ShortArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API IntegerArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API LongArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API FloatArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API DoubleArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API StringArraySerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API HazelcastJsonValueSerializer : public StreamSerializer {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const void *object);

                    virtual void *read(ObjectDataInput &in);
                };
            }
        }
    }
}


