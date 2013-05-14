//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CONSTANT_SERIALIZERS
#define HAZELCAST_CONSTANT_SERIALIZERS

#include "BufferedDataInput.h"
#include "BufferedDataOutput.h"
#include "SerializationConstants.h"

#include <iostream>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

//            namespace ConstantSerializers {

            template<typename T>
            int getTypeId(T t) {
                return 0;
            };


            inline int getTypeId(int t) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getTypeId(char c) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getTypeId(std::string t) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getTypeId(std::vector<byte> c) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            /*****************************************************************************/
            /*****************************************************************************/

//            template<typename BufferedDataOutput, typename T> //TODO did not work i just tried to overload every << calls :)
//            void writePortable(BufferedDataOutput& dataOutput, T data) {
//                dataOutput.writePortable(data);
//            };
//
//            template<typename BufferedDataInput, typename T>
//            void readPortable(BufferedDataInput& dataInput, T& data) {
//                data = dataInput.readPortable();
//            };

            /*****************************************************************************/
            inline int getFactoryId(byte t) {
                return -1;
            }

            inline int getClassId(byte t) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, byte data) {
                dataOutput.writeByte(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            /*****************************************************************************/
//                class BooleanSerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
//                    };
//
//                    void write(BufferedDataOutput *dataOutput, bool obj) {
//                        dataOutput->writeBoolean(obj);
//                    };
//
//                    bool read(BufferedDataInput *dataInput) {
//                        return dataInput->readBoolean();
//                    }
//                };

            /*****************************************************************************/
            inline int getFactoryId(char t) {
                return -1;
            }

            inline int getClassId(char t) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const char& data) {
                dataOutput.writeChar(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, char& data) {
                data = dataInput.readChar();
            };
            /*****************************************************************************/
//
//                class ShortSerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_SHORT;
//                    }
//
//                    short read(BufferedDataInput *dataInput) {
//                        return dataInput->readShort();
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, const short obj) {
//                        dataOutput->writeShort(obj);
//                    }
//                };
//
            /*****************************************************************************/
            inline int getFactoryId(int t) {
                return -1;
            }

            inline int getClassId(int t) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER;
            }

            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const int& data) {
                dataOutput.writeInt(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, int& data) {
                data = dataInput.readInt();
            };
            /*****************************************************************************/
//
//                class LongSerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_LONG;
//                    }
//
//                    long read(BufferedDataInput *dataInput) {
//                        return dataInput->readLong();
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, const long obj) {
//                        dataOutput->writeLong(obj);
//                    }
//                };
//
//                class FloatSerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_FLOAT;
//                    }
//
//                    float read(BufferedDataInput *dataInput) {
//                        return dataInput->readFloat();
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, const float obj) {
//                        dataOutput->writeFloat(obj);
//                    }
//                };
//
//                class DoubleSerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_DOUBLE;
//                    }
//
//                    double read(BufferedDataInput *dataInput) {
//                        return dataInput->readDouble();
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, const double obj) {
//                        dataOutput->writeDouble(obj);
//                    }
//                };
//
            /*****************************************************************************/
            inline int getFactoryId(std::string& obj) {
                return -1;
            };

            inline int getClassId(std::string& obj) {
                return SerializationConstants::CONSTANT_TYPE_STRING;
            };

            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const std::string&   data) {
                dataOutput.writeUTF(data);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            /*****************************************************************************/

            inline int getFactoryId(std::vector<byte>& obj) {
                return -1;
            };

            inline int getClassId(std::vector<byte>& obj) {
                return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            };

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
                    data[i] = dataInput->readByte();
                }
            };
            /*****************************************************************************/
//
//                class CharArraySerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
//                    }
//
//                    std::vector<char> read(BufferedDataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<char> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readChar();
//                        }
//                        return c;
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, std::vector<char>& obj) {
//                        int size = obj.size();
//                        dataOutput->writeInt(size);
//                        if (size > 0) {
//                            for (int i = 0; i < size; i++) {
//                                dataOutput->writeChar(obj[i]);
//                            }
//                        }
//                    }
//                };
//
//                class ShortArraySerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
//                    }
//
//                    std::vector<short> read(BufferedDataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<short> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readShort();
//                        }
//                        return c;
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, std::vector<short>& obj) {
//                        int size = obj.size();
//                        dataOutput->writeInt(size);
//                        if (size > 0) {
//                            for (int i = 0; i < size; i++) {
//                                dataOutput->writeShort(obj[i]);
//                            }
//                        }
//                    }
//                };
//
//                class IntegerArraySerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
//                    }
//
//                    std::vector<int> read(BufferedDataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<int> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readInt();
//                        }
//                        return c;
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, std::vector<int>& obj) {
//                        int size = obj.size();
//                        dataOutput->writeInt(size);
//                        if (size > 0) {
//                            for (int i = 0; i < size; i++) {
//                                dataOutput->writeInt(obj[i]);
//                            }
//                        }
//                    }
//                };
//
//                class LongArraySerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
//                    }
//
//                    std::vector<long> read(BufferedDataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<long> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readLong();
//                        }
//                        return c;
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, std::vector<long>& obj) {
//                        int size = obj.size();
//                        dataOutput->writeInt(size);
//                        if (size > 0) {
//                            for (int i = 0; i < size; i++) {
//                                dataOutput->writeLong(obj[i]);
//                            }
//                        }
//                    }
//                };
//
//                class FloatArraySerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
//                    }
//
//                    std::vector<float> read(BufferedDataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<float> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readShort();
//                        }
//                        return c;
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, std::vector<float>& obj) {
//                        int size = obj.size();
//                        dataOutput->writeInt(size);
//                        if (size > 0) {
//                            for (int i = 0; i < size; i++) {
//                                dataOutput->writeFloat(obj[i]);
//                            }
//                        }
//                    }
//                };
//
//                class DoubleArraySerializer {
//                public:
//
//                    inline int getClassId() {
//                        return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
//                    }
//
//                    std::vector<double> read(BufferedDataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<double> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readShort();
//                        }
//                        return c;
//                    }
//
//                    void write(BufferedDataOutput *dataOutput, std::vector<double>& obj) {
//                        int size = obj.size();
//                        dataOutput->writeInt(size);
//                        if (size > 0) {
//                            for (int i = 0; i < size; i++) {
//                                dataOutput->writeDouble(obj[i]);
//                            }
//                        }
//                    }
//                };

//            }

        }
    }
}
#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
