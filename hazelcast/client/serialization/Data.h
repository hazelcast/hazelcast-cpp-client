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
#include "ClassDefinition.h"
#include "SerializationContext.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class Data {
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

                bool operator ==(const Data&) const;

                bool operator !=(const Data&) const;

                void writeData(BufferedDataOutput&) const;

                void readData(BufferedDataInput&);

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
            inline void writePortable(DataOutput& dataOutput, const Data& data) {
                std::cout << "DATA WRITE PORTABLE IS CALLED" << std::endl;//TODO test
                dataOutput << data.type;
                if (data.cd != NULL) {
                    dataOutput << data.cd->getClassId();
                    dataOutput << data.cd->getFactoryId();
                    dataOutput << data.cd->getVersion();
                    std::vector<byte> classDefBytes = data.cd->getBinary();
                    dataOutput << classDefBytes.size();
                    dataOutput << classDefBytes;
                } else {
                    dataOutput << data.NO_CLASS_ID;
                }
                int len = data.bufferSize();
                dataOutput << len;
                if (len > 0) {
                    dataOutput << data.buffer;
                }
                dataOutput << data.partitionHash;
            };

            template<typename DataInput>
            inline void readPortable(DataInput& dataInput, Data& data) {
                std::cout << "DATA READ PORTABLE IS CALLED" << std::endl;//TODO test
                dataInput >> data.type;
                int classId = data.NO_CLASS_ID;
                dataInput >> classId;
                if (classId != data.NO_CLASS_ID) {
                    int factoryId = 0;
                    int version = 0;
                    dataInput >> factoryId;
                    dataInput >> version;

                    int classDefSize = 0;
                    dataInput >> classDefSize;

                    if (data.context->isClassDefinitionExists(factoryId, classId, version)) {
                        data.cd = data.context->lookup(factoryId, classId, version);
                        dataInput.skipBytes(classDefSize);//TODO msk ???
                    } else {
                        std::vector<byte> classDefBytes(classDefSize);
                        dataInput >> classDefBytes;
                        data.cd = data.context->createClassDefinition(factoryId, classDefBytes);
                    }
                }
                int size = 0;
                dataInput >> size;
                if (size > 0) {
                    std::vector<byte> buffer(size);
                    dataInput >> buffer;
                    data.buffer = buffer;
                }
                dataInput >> data.partitionHash;
            };

        }
    }
}
#endif /* HAZELCAST_DATA */
