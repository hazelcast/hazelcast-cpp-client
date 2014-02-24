//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include "hazelcast/client/serialization/pimpl/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/SerializationContext.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/util/HazelcastDll.h"
#include <vector>

namespace hazelcast {
    namespace client {
        namespace connection {

            class OutputSocketStream;

            class InputSocketStream;
        }
        namespace serialization {
            namespace pimpl {

                class HAZELCAST_API Data : public serialization::IdentifiedDataSerializable {
                public:

                    Data();

                    Data(const Data &);

                    Data &operator = (const Data &);

                    int bufferSize() const;

                    int totalSize() const;

                    int getPartitionHash() const;

                    void setPartitionHash(int);

                    int getType() const;

                    void setType(int type);

                    void setBuffer(std::auto_ptr< std::vector<byte> > buffer);

                    int hashCode() const;

                    void writeData(serialization::ObjectDataOutput &objectDataOutput) const;

                    void readData(serialization::ObjectDataInput &objectDataInput);

                    int getFactoryId() const;

                    int getClassId() const;

                    void writeToSocket(connection::OutputSocketStream &outputSocketStream) const;

                    void readFromSocket(connection::InputSocketStream &inputSocketStream);

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
}
#endif /* HAZELCAST_DATA */
