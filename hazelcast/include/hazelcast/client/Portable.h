//
// Created by sancar koyunlu on 6/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Portable
#define HAZELCAST_Portable

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;

            class PortableReader;
        }

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
         * @see IdentifiedDataSerializable
         */
        class HAZELCAST_API Portable {
        public:
            /**
             * Destructor
             */
            virtual ~Portable();

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
             * @param writer PortableWriter
             */
            virtual void writePortable(serialization::PortableWriter& writer) const = 0;

            /**
             *Defines how this class will be read.
             * @param reader PortableReader
             */
            virtual void readPortable(serialization::PortableReader& reader) = 0;

            /**
             * Not public api. Do not override this method.
             * @return serializer id
             */
            virtual int getSerializerId() const;
        };
    }

}


#endif //HAZELCAST_Portable
