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
// Created by sancar koyunlu on 6/7/13.

#ifndef HAZELCAST_TYPE_SERIALIZER
#define HAZELCAST_TYPE_SERIALIZER

#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            /**
             * Internal Base class for Serializers
             */
            class HAZELCAST_API SerializerBase {
            public:
                /**
                 * Destructor
                */
                virtual ~SerializerBase();

                /**
                 * unique type id for this serializer. It will be used to decide which serializer needs to be used
                 * for your classes. Also not that for your serialized classes you need to implement as free function
                 * in same namespace with your class
                 *
                 *      int32_t getHazelcastTypeId(const MyClass*);
                 *
                 *  which should return same id with its serializer.
                 */
                virtual int32_t getHazelcastTypeId() const = 0;

                /**
                 * The factory method to construct the custom objects
                 * Override this method if you want to provide a factory for the custom object.
                 * The memory should be managed correctly to avoid any leaks.
                 *
                 * @return The constructed object or NULL if no factory is provided.
                 */
                virtual void *create(ObjectDataInput &in) {
                    return 0;
                }
            };

            /**
             * Base class for custom serialization. If your all classes that needs to be serialized inherited from same
             * class you can use an implementation like following
             *

                    class  MyCustomSerializer : public serialization::Serializer<ExampleBaseClass> {
                         public:

                         void write(serialization::ObjectDataOutput & out, const ExampleBaseClass& object);

                         void read(serialization::ObjectDataInput & in, ExampleBaseClass& object);

                         int32_t getHazelcastTypeId() const;

                     };
                    }

             *
             * Or if they are not inherited from same base class you can use a serializer class like following
             * with templates.
             *

                    template<typename T>
                    class MyCustomSerializer : public serialization::Serializer<T> {
                    public:

                       void write(serialization::ObjectDataOutput & out, const T& object) {
                            //.....
                       }

                       void read(serialization::ObjectDataInput & in, T& object) {
                           //.....
                       }

                       int32_t getHazelcastTypeId() const {
                           //..
                       }
                    };

             *
             * Along with serializer following function should be provided with same namespace that ExampleBaseClass
             * belongs to
             *
             *     int32_t getHazelcastTypeId(const MyClass*);
             *
             *  which should return same id with its serializer.
             *
             * User than can register serializer via SerializationConfig as follows
             *

                   clientConfig.getSerializationConfig().registerSerializer(
                   boost::shared_ptr<hazelcast::client::serialization::SerializerBase>(new MyCustomSerializer());

             */
            template <typename Serializable>
            class Serializer : public SerializerBase {
            public:
                /**
                 * Destructor
                 */
                virtual ~Serializer() {}

                /**
                 *  This method writes object to ObjectDataOutput
                 *
                 *  @param out    ObjectDataOutput stream that object will be written to
                 *  @param object that will be written to out
                 */
                virtual void write(ObjectDataOutput &out, const Serializable &object) = 0;

                /**
                 *  Reads object from objectDataInputStream
                 *
                 *  @param in ObjectDataInput stream that object will read from
                 *  @param object read object
                 */
                virtual void read(ObjectDataInput &in, Serializable &object) = 0;
            };

        }
    }
}


#endif //HAZELCAST_TYPE_SERIALIZER

