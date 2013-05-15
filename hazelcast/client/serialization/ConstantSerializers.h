//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CONSTANT_SERIALIZERS
#define HAZELCAST_CONSTANT_SERIALIZERS

#include "SerializationConstants.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            template<typename T>
            int getTypeId(T t) {
                return 0;
            };

            template<typename T>
            int getFactoryId(T t) {
                return 0;
            }

            template<typename T>
            int getClassId(T t) {
                return 0;
            }

            inline int getTypeId(int t) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER;
            };

            inline int getTypeId(char c) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            };

            inline int getTypeId(std::string t) {
                return SerializationConstants::CONSTANT_TYPE_STRING;
            };

            inline int getTypeId(std::vector<byte> c) {
                return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            };
            /*****************************************************************************/
            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, byte data) {
                dataOutput.writeByte(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, bool data) {
                dataOutput.writeBoolean(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, bool& data) {
                data = dataInput.readBoolean();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const char& data) {
                dataOutput.writeChar(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, char& data) {
                data = dataInput.readChar();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, short data) {
                dataOutput.writeShort(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, short & data) {
                data = dataInput.readShort();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const int& data) {
                dataOutput.writeInt(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, int& data) {
                data = dataInput.readInt();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, long data) {
                dataOutput.writeLong(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, long & data) {
                data = dataInput.readLong();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, float data) {
                dataOutput.writeFloat(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, float & data) {
                data = dataInput.readFloat();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, double data) {
                dataOutput.writeDouble(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, double & data) {
                data = dataInput.readDouble();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::string&   data) {
                dataOutput.writeUTF(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<byte>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeByte(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<byte>& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readByte();
                }
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<char >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeChar(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<char >& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readChar();
                }
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<short >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeShort(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<short >& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readShort();
                }
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<int>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeInt(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<int>& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readInt();
                }
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<long >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeLong(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<long >& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readLong();
                }
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<float >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeFloat(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<float >& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readFloat();
                }
            };

            /*****************************************************************************/
            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::vector<double >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeDouble(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::vector<double >& data) {
                int size = dataInput.readInt();
                data.resize(size, 0);
                for (int i = 0; i < size; i++) {
                    data[i] = dataInput.readDouble();
                }
            };
            /*****************************************************************************/
        }
    }
}
#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
