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

        class HAZELCAST_API Portable {
        public:
            virtual ~Portable();

            virtual int getFactoryId() const = 0;

            virtual int getClassId() const = 0;

            virtual void writePortable(serialization::PortableWriter& writer) const = 0;

            virtual void readPortable(serialization::PortableReader& reader) = 0;

            virtual int getSerializerId() const;
        };
    }

}


#endif //HAZELCAST_Portable
