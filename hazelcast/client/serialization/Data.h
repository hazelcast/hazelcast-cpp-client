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

            class BufferedDataOutput;

            class BufferedDataInput;

            class ClassDefinition;

            class SerializationContext;

            class InputSocketStream;

            class OutputSocketStream;

            typedef unsigned char byte;

            class Data {
                template<typename DataOutput>
                friend void operator <<(DataOutput& dataOutput, const Data& data);

                template<typename DataInput>
                friend void operator >>(DataInput& dataInput, Data& data);

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

                int getClassId() const;
            };

            template<typename DataOutput>
            void operator <<(DataOutput& dataOutput, const Data& data);

            template<typename DataInput>
            void operator >>(DataInput& dataInput, Data& data);

        }
    }
}
#endif /* HAZELCAST_DATA */
