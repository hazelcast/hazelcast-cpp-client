//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CONSTANT_SERIALIZERS
#define HAZELCAST_CONSTANT_SERIALIZERS

#include "DataInput.h"
#include "DataOutput.h"
#include "SerializationConstants.h"

#include <iostream>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

//            namespace ConstantSerializers {

            template<typename T>
            int getTypeId(T t) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
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
            inline int getFactoryId(byte t) {
                return -1;
            }

            inline int getClassId(byte t) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            template<typename DataOutput>
            void operator <<(DataOutput& dataOutput, byte data) {
                dataOutput.writeByte(data);
            };

            template<typename DataInput>
            void operator >>(DataInput& dataInput, byte& data) {
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
//                    void write(DataOutput *dataOutput, bool obj) {
//                        dataOutput->writeBoolean(obj);
//                    };
//
//                    bool read(DataInput *dataInput) {
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
            void operator <<(DataOutput& dataOutput, const char& data) {
                dataOutput.writeChar(data);
            };

            template<typename DataInput>
            void operator >>(DataInput& dataInput, char& data) {
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
//                    short read(DataInput *dataInput) {
//                        return dataInput->readShort();
//                    }
//
//                    void write(DataOutput *dataOutput, const short obj) {
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
            void operator <<(DataOutput& dataOutput, const int& data) {
                dataOutput.writeInt(data);
            };

            template<typename DataInput>
            void operator >>(DataInput& dataInput, int& data) {
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
//                    long read(DataInput *dataInput) {
//                        return dataInput->readLong();
//                    }
//
//                    void write(DataOutput *dataOutput, const long obj) {
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
//                    float read(DataInput *dataInput) {
//                        return dataInput->readFloat();
//                    }
//
//                    void write(DataOutput *dataOutput, const float obj) {
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
//                    double read(DataInput *dataInput) {
//                        return dataInput->readDouble();
//                    }
//
//                    void write(DataOutput *dataOutput, const double obj) {
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
            void operator <<(DataOutput& dataOutput, const std::string&   data) {
                dataOutput.writeUTF(data);
            };

            template<typename DataInput>
            void operator >>(DataInput& dataInput, std::string&  data) {
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
            void operator <<(DataOutput& dataOutput, const std::vector<byte>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeByte(data[i]);
                    }
                }
            };

            template<typename DataInput>
            void operator >>(DataInput& dataInput, std::vector<byte>& data) {
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
//                    std::vector<char> read(DataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<char> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readChar();
//                        }
//                        return c;
//                    }
//
//                    void write(DataOutput *dataOutput, std::vector<char>& obj) {
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
//                    std::vector<short> read(DataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<short> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readShort();
//                        }
//                        return c;
//                    }
//
//                    void write(DataOutput *dataOutput, std::vector<short>& obj) {
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
//                    std::vector<int> read(DataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<int> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readInt();
//                        }
//                        return c;
//                    }
//
//                    void write(DataOutput *dataOutput, std::vector<int>& obj) {
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
//                    std::vector<long> read(DataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<long> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readLong();
//                        }
//                        return c;
//                    }
//
//                    void write(DataOutput *dataOutput, std::vector<long>& obj) {
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
//                    std::vector<float> read(DataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<float> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readShort();
//                        }
//                        return c;
//                    }
//
//                    void write(DataOutput *dataOutput, std::vector<float>& obj) {
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
//                    std::vector<double> read(DataInput *dataInput) {
//                        int size = dataInput->readInt();
//                        std::vector<double> c(size);
//                        for (int i = 0; i < size; i++) {
//                            c[i] = dataInput->readShort();
//                        }
//                        return c;
//                    }
//
//                    void write(DataOutput *dataOutput, std::vector<double>& obj) {
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
