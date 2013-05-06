//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include "DataSerializable.h"
#include "IdentifiedDataSerializable.h"
#include <vector>
#include <iostream>
#include <memory>
#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataOutput;

            class DataInput;

            class ClassDefinition;

            typedef unsigned char byte;

            class Data : public IdentifiedDataSerializable {
            public:

                Data();

                Data(const Data&);

                Data(const int type, std::vector<byte> bytes);

                ~Data();


                Data& operator = (const Data&);

                int bufferSize() const;

                int totalSize() const;

                int getPartitionHash();

                void setPartitionHash(int partitionHash);

                bool operator ==(const Data&) const;

                bool operator !=(const Data&) const;

                void writeData(DataOutput&) const;

                void readData(DataInput&);

                boost::shared_ptr<ClassDefinition> cd;
                int type;
                std::vector<byte> buffer;
                static int const NO_CLASS_ID = 0;
            private:
                int partitionHash;
                static int const FACTORY_ID = 0;
                static int const ID = 0;

                virtual int getFactoryId() const;

                virtual int getId() const;
            };

        }
    }
}
#endif /* HAZELCAST_DATA */
