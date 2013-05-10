//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include <vector>
#include <iosfwd>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataOutput;

            class DataInput;

            class ClassDefinition;

            class SerializationContext;

            class InputSocketStream;

            class OutputSocketStream;

            typedef unsigned char byte;

            class Data {
            public:

                Data();

                Data(const Data&);

                Data(const int type, std::vector<byte> bytes);

                ~Data();


                Data& operator = (const Data&);

                void setSerializationContext(SerializationContext *context);

                int bufferSize() const;

                int totalSize() const;

                int getPartitionHash();

                void setPartitionHash(int partitionHash);

                bool operator ==(const Data&) const;

                bool operator !=(const Data&) const;

                void writeData(DataOutput&) const;

                void readData(DataInput&);

                void writeData(OutputSocketStream&) const;

                void readData(InputSocketStream&);

                boost::shared_ptr<ClassDefinition> cd;
                int type;
                std::vector<byte> buffer;
                static int const NO_CLASS_ID = 0;
            private:
                SerializationContext *context;

                int partitionHash;
                static int const FACTORY_ID = 0;
                static int const ID = 0;

                int getFactoryId() const;

                int getId() const;
            };

        }
    }
}
#endif /* HAZELCAST_DATA */
