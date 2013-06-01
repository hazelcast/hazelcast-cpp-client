//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantPortableWriter.h"
#include "../PortableWriter.h"
#include "../Data.h"
#include "../NullPortable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            void writePortable(PortableWriter& dataOutput, byte data) {
                dataOutput.writeByte(data);
            };

            void writePortable(PortableWriter& dataOutput, bool data) {
                dataOutput.writeBoolean(data);
            };


            void writePortable(PortableWriter& dataOutput, char data) {
                dataOutput.writeChar(data);
            };


            void writePortable(PortableWriter& dataOutput, short data) {
                dataOutput.writeShort(data);
            };

            void writePortable(PortableWriter& dataOutput, int data) {
                dataOutput.writeInt(data);
            };

            void writePortable(PortableWriter& dataOutput, long data) {
                dataOutput.writeLong(data);
            };

            void writePortable(PortableWriter& dataOutput, float data) {
                dataOutput.writeFloat(data);
            };

            void writePortable(PortableWriter& dataOutput, double data) {
                dataOutput.writeDouble(data);
            };

            void writePortable(PortableWriter& dataOutput, const std::string&   data) {
                dataOutput.writeUTF(data);
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<byte>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeByte(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<char >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeChar(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<short >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeShort(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<int>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeInt(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<long >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeLong(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<float >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeFloat(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const std::vector<double >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeDouble(data[i]);
                    }
                }
            };

            void writePortable(PortableWriter& dataOutput, const NullPortable&  data) {
                dataOutput.writeNullPortable(data.factoryId, data.classId);
            };


            void writePortable(PortableWriter& dataOutput, const Data&  data) {
                dataOutput.writeInt(data.type);
                if (data.cd != NULL) {
                    dataOutput.writeInt(data.cd->getClassId());
                    dataOutput.writeInt(data.cd->getFactoryId());
                    dataOutput.writeInt(data.cd->getVersion());
                    const std::vector<byte>& classDefBytes = data.cd->getBinary();
                    dataOutput.writeInt(classDefBytes.size());
                    dataOutput.write(classDefBytes);
                } else {
                    dataOutput.writeInt(data.NO_CLASS_ID);
                }
                int len = data.bufferSize();
                dataOutput.writeInt(len);
                if (len > 0) {
                    dataOutput.write(*(data.buffer.get()));
                }
                dataOutput.writeInt(data.partitionHash);
            };


            void operator <<(PortableWriter& dataOutput, byte data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, bool data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, char data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, short data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, int data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, long data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, float data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, double data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::string&   data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<byte>&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<char >&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<short >&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<int>&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<long >&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<float >&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const std::vector<double >&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const NullPortable&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

            void operator <<(PortableWriter& dataOutput, const Data&  data) {
                dataOutput.writingToDataOutput();
                writePortable(dataOutput, data);
            };

        }
    }
}
