//
// Created by sancar koyunlu on 6/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_Portable
#define HAZELCAST_Portable

namespace hazelcast {
    namespace client {

        typedef unsigned char byte;

        namespace serialization {
            class PortableWriter;

            class PortableReader;
        }

        class Portable {
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
