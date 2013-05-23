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
#include "DataSerializable.h"
#include <vector>
#include <iosfwd>


namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class Data : public DataSerializable{
                template<typename DataOutput>
                friend void writePortable(DataOutput& dataOutput, const Data& data);

                template<typename DataInput>
                friend void readPortable(DataInput& dataInput, Data& data);

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

                bool isServerError();

                bool operator ==(const Data&) const;

                bool operator !=(const Data&) const;

                template<typename  Out>
                void writeData(Out & dataOutput) const {
                    dataOutput.writeInt(type);
                    if (cd != NULL) {
                        dataOutput.writeInt(cd->getClassId());
                        dataOutput.writeInt(cd->getFactoryId());
                        dataOutput.writeInt(cd->getVersion());
                        std::vector<byte> classDefBytes = cd->getBinary();
                        dataOutput.writeInt(classDefBytes.size());
                        dataOutput.write(classDefBytes);
                    } else {
                        dataOutput.writeInt(NO_CLASS_ID);
                    }
                    int len = bufferSize();
                    dataOutput.writeInt(len);
                    if (len > 0) {
                        dataOutput.write(buffer);
                    }
                    dataOutput.writeInt(partitionHash);

                }

                template<typename  Input>
                void readData(Input & dataInput) {
                    type = dataInput.readInt();
                    int classId = dataInput.readInt();

                    if (classId != NO_CLASS_ID) {
                        int factoryId = dataInput.readInt();
                        isError = (factoryId == hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY)
                                && (classId == hazelcast::client::protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID);
                        int version = dataInput.readInt();

                        int classDefSize = dataInput.readInt();

                        if (context->isClassDefinitionExists(factoryId, classId, version)) {
                            cd = context->lookup(factoryId, classId, version);
                            dataInput.skipBytes(classDefSize);
                        } else {
                            std::vector<byte> classDefBytes(classDefSize);
                            dataInput.readFully(classDefBytes);
                            cd = context->createClassDefinition(factoryId, classDefBytes);
                        }
                    }
                    int size = dataInput.readInt();
                    if (size > 0) {
                        this->buffer.resize(size, 0);
                        dataInput.readFully(buffer);
                    }
                    partitionHash = dataInput.readInt();
                }

                ClassDefinition *cd;
                int type;
                std::vector<byte> buffer;
                static int const NO_CLASS_ID = 0;
            private:
                SerializationContext *context;
                bool isError;
                int partitionHash;
                static int const FACTORY_ID = 0;
                static int const ID = 0;

                int getFactoryId() const;

                int getClassId() const;
            };

            template<typename HzWriter>
            inline void writePortable(HzWriter& dataOutput, const Data& data) {
                dataOutput.writeInt(data.type);
                if (data.cd != NULL) {
                    dataOutput.writeInt(data.cd->getClassId());
                    dataOutput.writeInt(data.cd->getFactoryId());
                    dataOutput.writeInt(data.cd->getVersion());
                    std::vector<byte> classDefBytes = data.cd->getBinary();
                    dataOutput.writeInt(classDefBytes.size());
                    dataOutput.write(classDefBytes);
                } else {
                    dataOutput.writeInt(data.NO_CLASS_ID);
                }
                int len = data.bufferSize();
                dataOutput.writeInt(len);
                if (len > 0) {
                    dataOutput.write(data.buffer);
                }
                dataOutput.writeInt(data.partitionHash);
            };

            template<typename HzReader>
            inline void readPortable(HzReader& dataInput, Data& data) {
                data.type = dataInput.readInt();
                int classId = dataInput.readInt();

                if (classId != data.NO_CLASS_ID) {
                    int factoryId = dataInput.readInt();
                    data.isError = (factoryId == hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY)
                            && (classId == hazelcast::client::protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID);
                    int version = dataInput.readInt();

                    int classDefSize = dataInput.readInt();

                    if (data.context->isClassDefinitionExists(factoryId, classId, version)) {
                        data.cd = data.context->lookup(factoryId, classId, version);
                        dataInput.skipBytes(classDefSize);
                    } else {
                        std::vector<byte> classDefBytes(classDefSize);
                        dataInput.readFully(classDefBytes);
                        data.cd = data.context->createClassDefinition(factoryId, classDefBytes);
                    }
                }
                int size = dataInput.readInt();
                if (size > 0) {
                    data.buffer.resize(size, 0);
                    dataInput.readFully(data.buffer);
                }
                data.partitionHash = dataInput.readInt();
            };

        }
    }
}
#endif /* HAZELCAST_DATA */
