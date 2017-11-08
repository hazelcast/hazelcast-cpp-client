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
#include <memory>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            /**
             * Implement this interface and register to the SerializationConfig. See examples folder for usage examples.
             *
             * Important note:
             * you need to implement as free function in same namespace with your class
             *            int32_t getHazelcastTypeId(const MyClass*);
             *
             * which should return same id with its serializer.             *
             *
             */
            class StreamSerializer {
            public:
                virtual ~StreamSerializer();

                /**
                 * unique type id for this serializer. It will be used to decide which serializer needs to be used
                 * for your classes. Also note that for your serialized classes you need to implement as free function
                 * in same namespace with your class
                 *
                 *      int32_t getHazelcastTypeId(const MyClass*);
                 *
                 *  which should return same id with its serializer.
                 */
                virtual int32_t getHazelcastTypeId() const = 0;

                /**
                 *  This method writes object to ObjectDataOutput
                 *
                 *  @param out    ObjectDataOutput stream that object will be written to
                 *  @param object that will be written to out
                 */
                virtual void write(ObjectDataOutput &out, const void *object) = 0;

                /**
                 * The factory method to construct the custom objects
                 * Override this method if you want to provide a factory for the custom object.
                 * The memory should be managed correctly to avoid any leaks.
                 *
                 * @param in The input stream to be read.
                 * @return The constructed object.
                 */
                virtual void *read(ObjectDataInput &in) = 0;
            };

            /**
             * @deprecated Please use StreamSerializer for custom Serialization
             */
            template <typename T>
            class Serializer : public StreamSerializer {
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
                virtual void write(ObjectDataOutput &out, const T &object) = 0;

                /**
                 *  Reads object from objectDataInputStream
                 *
                 *  @param in ObjectDataInput stream that object will read from
                 *  @param object read object from input data
                 */
                virtual void read(ObjectDataInput &in, T &object) = 0;

                /**
                 * This is an internal method for backward compatibility.
                 * @param in ObjectDataInput stream that object will read from
                 * @return read object from input data
                 */
                virtual void *read(ObjectDataInput &in) {
                    std::auto_ptr<T> object(new T);
                    read(in, *object);
                    return object.release();
                }

                virtual void write(ObjectDataOutput &out, const void *object) {
                    write(out, *(static_cast<const T *>(object)));
                }
            };

        }
    }
}


#endif //HAZELCAST_TYPE_SERIALIZER

