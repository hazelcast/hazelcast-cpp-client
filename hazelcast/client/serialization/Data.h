//
//  Data.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA
#define HAZELCAST_DATA

#include "ClassDefinition.h"
#include "SerializationContext.h"
#include "../protocol/ProtocolConstants.h"
#include "ConstantSerializers.h"
#include "../HazelcastException.h"
#include <vector>
#include <iosfwd>


namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class Data {
                friend void readPortable(PortableReader& dataInput, Data& data);

                friend void readPortable(MorphingPortableReader& portableReader, Data& data);

            public:

                Data();

                Data(const int type, std::auto_ptr <std::vector<byte> > bytes);

                Data(const Data&);

                Data& operator = (const Data&);

                ~Data();

                int bufferSize() const;

                int totalSize() const;

                int getPartitionHash();

                void setPartitionHash(int partitionHash);

                int getType();

                void setType(int type);

                void setBuffer(std::auto_ptr< std::vector<byte> > buffer);

                bool isServerError() const;

                int hashCode();

                bool operator ==(const Data&) const;

                bool operator !=(const Data&) const;

                template<typename  Out>
                void writeData(Out & dataOutput) const {
                    dataOutput.writeInt(type);
                    if (cd != NULL) {
                        dataOutput.writeInt(cd->getClassId());
                        dataOutput.writeInt(cd->getFactoryId());
                        dataOutput.writeInt(cd->getVersion());
                        const std::vector<byte>& classDefBytes = cd->getBinary();

                        dataOutput.writeInt(classDefBytes.size());
                        dataOutput.write(classDefBytes);
                    } else {
                        dataOutput.writeInt(NO_CLASS_ID);
                    }
                    int len = bufferSize();
                    dataOutput.writeInt(len);
                    if (len > 0) {
                        dataOutput.write(*(buffer.get()));
                    }
                    dataOutput.writeInt(partitionHash);

                }

                template<typename  Input>
                void readData(Input & dataInput, SerializationContext& serializationContext) {
                    type = dataInput.readInt();
                    int classId = dataInput.readInt();

                    if (classId != NO_CLASS_ID) {
                        int factoryId = dataInput.readInt();
                        isError = (factoryId == hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY)
                                && (classId == hazelcast::client::protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID);
                        int version = dataInput.readInt();

                        int classDefSize = dataInput.readInt();

                        if (serializationContext.isClassDefinitionExists(factoryId, classId, version)) {
                            cd = serializationContext.lookup(factoryId, classId, version);
                            dataInput.skipBytes(classDefSize);
                        } else {
                            std::auto_ptr< std::vector<byte> > classDefBytes (new std::vector<byte> (classDefSize));
                            dataInput.readFully(*(classDefBytes.get()));
                            cd = serializationContext.createClassDefinition(factoryId, classDefBytes);
                        }
                    }
                    int size = dataInput.readInt();
                    if (size > 0) {
                        this->buffer->resize(size, 0);
                        dataInput.readFully(*(buffer.get()));
                    }
                    partitionHash = dataInput.readInt();
                }

                template<typename  Input>
                void readData(Input & dataInput) {
                    type = dataInput.readInt();
                    int classId = dataInput.readInt();

                    if (classId != NO_CLASS_ID) {
                        throw client::HazelcastException("It is not pure data");
                    }
                    int size = dataInput.readInt();
                    if (size > 0) {
                        this->buffer->resize(size, 0);
                        dataInput.readFully(*(buffer.get()));
                    }
                    partitionHash = dataInput.readInt();
                }

                ClassDefinition *cd;
                int type;
                mutable std::auto_ptr< std::vector<byte> > buffer;
                static int const NO_CLASS_ID = 0;
                int partitionHash;


            private:

                bool isError;
                static int const FACTORY_ID = 0;
                static int const ID = 0;


                int getFactoryId() const;

                int getClassId() const;
            };


            inline int getTypeSerializerId(const Data& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

        }
    }
}
#endif /* HAZELCAST_DATA */
