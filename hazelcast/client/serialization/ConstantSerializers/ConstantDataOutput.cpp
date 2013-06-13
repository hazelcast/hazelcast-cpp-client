//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantDataOutput.h"
#include "../BufferedDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            void operator <<(BufferedDataOutput& dataOutput, byte data) {
                dataOutput.writeByte(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, bool data) {
                dataOutput.writeBoolean(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, char data) {
                dataOutput.writeChar(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, short data) {
                dataOutput.writeShort(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, int data) {
                dataOutput.writeInt(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, long data) {
                dataOutput.writeLong(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, float data) {
                dataOutput.writeFloat(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, double data) {
                dataOutput.writeDouble(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::string&   data) {
                dataOutput.writeUTF(data);
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<byte>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; ++i) {
                        dataOutput.writeByte(data[i]);
                    }
                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<char >&  data) {
                dataOutput.writeCharArray(data);
//                int size = data.size();
//                dataOutput.writeInt(size);
//                if (size > 0) {
//                    for (int i = 0; i < size; ++i) {
//                        dataOutput.writeChar(data[i]);
//                    }
//                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<short >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; ++i) {
                        dataOutput.writeShort(data[i]);
                    }
                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<int>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; ++i) {
                        dataOutput.writeInt(data[i]);
                    }
                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<long >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; ++i) {
                        dataOutput.writeLong(data[i]);
                    }
                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<float >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; ++i) {
                        dataOutput.writeFloat(data[i]);
                    }
                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<double >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; ++i) {
                        dataOutput.writeDouble(data[i]);
                    }
                }
            };

            void operator <<(BufferedDataOutput& dataOutput, const NullPortable&  data) {
                throw hazelcast::client::HazelcastException("Write null portable to BufferedDataOutput is not supported!");
            };

            void operator <<(BufferedDataOutput& dataOutput, const Data& data) {

            };
        }
    }
}
