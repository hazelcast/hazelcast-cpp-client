//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/util/HazelcastDll.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class OutputSocketStream;

            class InputSocketStream;

            class HAZELCAST_API Data : public IdentifiedDataSerializable {
            public:

                Data();

                Data(const Data &);

                Data &operator = (const Data &);

                int bufferSize() const;

                int totalSize() const;

                int getPartitionHash() const;

                void setPartitionHash(int );

                int getType() const;

                void setType(int type);

                void setBuffer(std::auto_ptr< std::vector<byte> > buffer);

                int hashCode() const;

                void writeData(serialization::ObjectDataOutput &objectDataOutput) const;

                void readData(serialization::ObjectDataInput &objectDataInput);

                int getFactoryId() const;

                int getClassId() const;

                void writeToSocket(serialization::OutputSocketStream &outputSocketStream) const;

                void readFromSocket(serialization::InputSocketStream &inputSocketStream);

                boost::shared_ptr<ClassDefinition> cd;
                mutable std::auto_ptr< std::vector<byte> > buffer;
                static int const NO_CLASS_ID = 0;
            private:
                mutable int partitionHash;
                int type;
                static int const FACTORY_ID = 0;
                static int const CLASS_ID = 0;
            };
        }
    }
}
#endif /* HAZELCAST_DATA */
