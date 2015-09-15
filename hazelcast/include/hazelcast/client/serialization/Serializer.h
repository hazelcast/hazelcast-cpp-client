//
// Created by sancar koyunlu on 6/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TYPE_SERIALIZER
#define HAZELCAST_TYPE_SERIALIZER

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
                 * for your classes. Also not that your serialized classes needs to implement
                 *
                 *      int getTypeId();
                 *
                 *  which should return same id with its serializer.
                 */
                virtual int getTypeId() const = 0;
            };

            /**
             * Base class for custom serialization. If your all classes that needs to be serialized inherited from same
             * class you can use an implementation like following
             *

                    class  MyCustomSerializer : public serialization::Serializer<ExampleBaseClass> {
                         public:

                         void write(serialization::ObjectDataOutput & out, const ExampleBaseClass& object);

                         void read(serialization::ObjectDataInput & in, ExampleBaseClass& object);

                         int getTypeId() const;

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

                       int getTypeId() const {
                           //..
                       }
                    };

             *
             * User than can register serializer via SerializationConfig as follows
             *

                   clientConfig.getSerializationConfig().registerSerializer(new MyCustomSerializer());

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

