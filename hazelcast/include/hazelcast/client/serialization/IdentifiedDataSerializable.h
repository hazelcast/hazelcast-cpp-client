//
// Created by sancar koyunlu on 6/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_IdentifiedDataSerializable
#define HAZELCAST_IdentifiedDataSerializable

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            /**
             * Classes that will be used with hazelcast data structures like IMap, IQueue etc. should inherit from
             * one of the following classes
             * Portable
             * IdentifiedDataSerializable
             *
             * Note that: factoryId and classId is to the same class in find server side, hence they should be consistent
             * with each other.
             *
             *
             *
             * @see Portable
             */
            class HAZELCAST_API IdentifiedDataSerializable {
            public:
                /**
                 * Destructor
                 */
                virtual ~IdentifiedDataSerializable();

                /**
                 * @return factory id
                 */
                virtual int getFactoryId() const = 0;

                /**
                 * @return class id
                 */
                virtual int getClassId() const = 0;

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                virtual void writeData(ObjectDataOutput &writer) const = 0;

                /**
                 *Defines how this class will be read.
                 * @param reader ObjectDataInput
                 */
                virtual void readData(ObjectDataInput &reader) = 0;

                /**
                 * Not public api. Do not override this method.
                 * @return serializer id
                 */
                virtual int getSerializerId() const;

            };
        }

    }
}

#endif //HAZELCAST_DataSerializable

