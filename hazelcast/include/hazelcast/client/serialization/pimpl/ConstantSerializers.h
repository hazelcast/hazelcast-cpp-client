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
                class HAZELCAST_API NullSerializer : public StreamSerializer<byte> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const byte &object);

                    virtual void *read(ObjectDataInput &in);
                };

                class HAZELCAST_API ByteSerializer : public Serializer<byte> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const byte &object);

                    virtual void read(ObjectDataInput &in, byte &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API BooleanSerializer : public Serializer<bool> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const bool &object);

                    virtual void *create(ObjectDataInput &in);

                    virtual void read(ObjectDataInput &in, bool &object);
                };

                class HAZELCAST_API CharSerializer : public Serializer<char> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const char &object);

                    virtual void read(ObjectDataInput &in, char &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API ShortSerializer : public Serializer<int16_t> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const int16_t &object);

                    virtual void read(ObjectDataInput &in, int16_t &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API IntegerSerializer : public Serializer<int32_t> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const int32_t &object);

                    virtual void read(ObjectDataInput &in, int32_t &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API LongSerializer : public Serializer<int64_t> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const int64_t &object);

                    virtual void read(ObjectDataInput &in, int64_t &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API FloatSerializer : public Serializer<float> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const float &object);

                    virtual void read(ObjectDataInput &in, float &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API DoubleSerializer : public Serializer<double> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const double &object);

                    virtual void read(ObjectDataInput &in, double &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API StringSerializer : public Serializer<std::string> {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::string &object);

                    virtual void read(ObjectDataInput &in, std::string &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API TheByteArraySerializer : public Serializer<std::vector<byte> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<byte> &object);

                    virtual void read(ObjectDataInput &in, std::vector<byte> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API BooleanArraySerializer : public Serializer<std::vector<bool> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<bool> &object);

                    virtual void read(ObjectDataInput &in, std::vector<bool> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API CharArraySerializer : public Serializer<std::vector<char> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<char> &object);

                    virtual void read(ObjectDataInput &in, std::vector<char> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API ShortArraySerializer : public Serializer<std::vector<int16_t> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<int16_t> &object);

                    virtual void read(ObjectDataInput &in, std::vector<int16_t> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API IntegerArraySerializer : public Serializer<std::vector<int32_t> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<int32_t> &object);

                    virtual void read(ObjectDataInput &in, std::vector<int32_t> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API LongArraySerializer : public Serializer<std::vector<int64_t> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<int64_t> &object);

                    virtual void read(ObjectDataInput &in, std::vector<int64_t> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API FloatArraySerializer : public Serializer<std::vector<float> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<float> &object);

                    virtual void read(ObjectDataInput &in, std::vector<float> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API DoubleArraySerializer : public Serializer<std::vector<double > > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<double> &object);

                    virtual void read(ObjectDataInput &in, std::vector<double> &object);

                    virtual void *create(ObjectDataInput &in);
                };

                class HAZELCAST_API StringArraySerializer : public Serializer<std::vector<std::string> > {
                public:
                    virtual int32_t getHazelcastTypeId() const;

                    virtual void write(ObjectDataOutput &out, const std::vector<std::string> &object);

                    virtual void read(ObjectDataInput &in, std::vector<std::string> &object);

                    virtual void *create(ObjectDataInput &in);
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_SERIALIZATION_PIMPL_CANSTANTSERIALIZERS_H_

