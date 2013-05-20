#include "ConstantSerializers.h"
#include "ClassDefinitionWriter.h"
#include "MorphingPortableReader.h"
#include "BufferedDataOutput.h"
#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            int getTypeId(byte data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getTypeId(bool data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getTypeId(char data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            };

            int getTypeId(short data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT;
            };

            int getTypeId(int data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER;
            };

            int getTypeId(long data) {
                return SerializationConstants::CONSTANT_TYPE_LONG;
            };

            int getTypeId(float data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT;
            };

            int getTypeId(double data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE;
            };

            int getTypeId(const std::string&   data) {
                return SerializationConstants::CONSTANT_TYPE_STRING;
            };

            int getTypeId(const std::vector<byte>&  data) {
                return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            };

            int getTypeId(const std::vector<char >&  data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            };

            int getTypeId(const std::vector<short >&  data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            };

            int getTypeId(const std::vector<int>&  data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            };

            int getTypeId(const std::vector<long >&  data) {
                return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            };

            int getTypeId(const std::vector<float >&  data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            };

            int getTypeId(const std::vector<double >&  data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            };

            /*****************************************************************************/
            /*****************************************************************************/
            void writePortable(ClassDefinitionWriter& cdw, byte data) {
                cdw.writeByte(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, bool data) {
                cdw.writeBoolean(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, char data) {
                cdw.writeChar(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, short data) {
                cdw.writeShort(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, int data) {
                cdw.writeInt(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, long data) {
                cdw.writeLong(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, float data) {
                cdw.writeFloat(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, double data) {
                cdw.writeDouble(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::string&   data) {
                cdw.writeUTF(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<byte>&  data) {
                cdw.writeByteArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<char >&  data) {
                cdw.writeCharArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<short >&  data) {
                cdw.writeShortArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<int>&  data) {
                cdw.writeIntArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<long >&  data) {
                cdw.writeLongArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<float >&  data) {
                cdw.writeFloatArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<double >&  data) {
                cdw.writeDoubleArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const NullPortable&  data) {
                cdw.writeNullPortable(data.factoryId, data.classId);
            };
            /*****************************************************************************/
            /*****************************************************************************/
            void writePortable(BufferedDataOutput& dataOutput, byte data) {
                dataOutput.writeByte(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, bool data) {
                dataOutput.writeBoolean(data);
            };


            void writePortable(BufferedDataOutput& dataOutput, char data) {
                dataOutput.writeChar(data);
            };


            void writePortable(BufferedDataOutput& dataOutput, short data) {
                dataOutput.writeShort(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, int data) {
                dataOutput.writeInt(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, long data) {
                dataOutput.writeLong(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, float data) {
                dataOutput.writeFloat(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, double data) {
                dataOutput.writeDouble(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::string&   data) {
                dataOutput.writeUTF(data);
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<byte>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeByte(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<char >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeChar(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<short >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeShort(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<int>&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeInt(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<long >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeLong(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<float >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeFloat(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<double >&  data) {
                int size = data.size();
                dataOutput.writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        dataOutput.writeDouble(data[i]);
                    }
                }
            };

            void writePortable(BufferedDataOutput& dataOutput, const NullPortable&  data) {
                throw hazelcast::client::HazelcastException("Write null portable to BufferedDataOutput is not supported!");
            };
            /*****************************************************************************/
            /*****************************************************************************/
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
            /*****************************************************************************/
            /*****************************************************************************/
            void readPortable(PortableReader& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            void readPortable(PortableReader& dataInput, bool& data) {
                data = dataInput.readBoolean();
            };

            void readPortable(PortableReader& dataInput, char& data) {
                data = dataInput.readChar();
            };

            void readPortable(PortableReader& dataInput, short & data) {
                data = dataInput.readShort();
            };

            void readPortable(PortableReader& dataInput, int& data) {
                data = dataInput.readInt();
            };

            void readPortable(PortableReader& dataInput, long & data) {
                data = dataInput.readLong();
            };

            void readPortable(PortableReader& dataInput, float & data) {
                data = dataInput.readFloat();
            };

            void readPortable(PortableReader& dataInput, double & data) {
                data = dataInput.readDouble();
            };

            void readPortable(PortableReader& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            void readPortable(PortableReader& dataInput, std::vector<byte>& data) {
                data = dataInput.readByteArray();
            };

            void readPortable(PortableReader& dataInput, std::vector<char >& data) {
                data = dataInput.readCharArray();
            };

            void readPortable(PortableReader& dataInput, std::vector<short >& data) {
                data = dataInput.readShortArray();
            };

            void readPortable(PortableReader& dataInput, std::vector<int>& data) {
                data = dataInput.readIntArray();
            };

            void readPortable(PortableReader& dataInput, std::vector<long >& data) {
                data = dataInput.readLongArray();
            };

            void readPortable(PortableReader& dataInput, std::vector<float >& data) {
                data = dataInput.readFloatArray();
            };

            void readPortable(PortableReader& dataInput, std::vector<double >& data) {
                data = dataInput.readDoubleArray();
            }

            /*****************************************************************************/
            void readPortable(MorphingPortableReader& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            void readPortable(MorphingPortableReader& dataInput, bool& data) {
                data = dataInput.readBoolean();
            };

            void readPortable(MorphingPortableReader& dataInput, char& data) {
                data = dataInput.readChar();
            };

            void readPortable(MorphingPortableReader& dataInput, short & data) {
                data = dataInput.readShort();
            };

            void readPortable(MorphingPortableReader& dataInput, int& data) {
                data = dataInput.readInt();
            };

            void readPortable(MorphingPortableReader& dataInput, long & data) {
                data = dataInput.readLong();
            };

            void readPortable(MorphingPortableReader& dataInput, float & data) {
                data = dataInput.readFloat();
            };

            void readPortable(MorphingPortableReader& dataInput, double & data) {
                data = dataInput.readDouble();
            };

            void readPortable(MorphingPortableReader& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<byte>& data) {
                data = dataInput.readByteArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<char >& data) {
                data = dataInput.readCharArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<short >& data) {
                data = dataInput.readShortArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<int>& data) {
                data = dataInput.readIntArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<long >& data) {
                data = dataInput.readLongArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<float >& data) {
                data = dataInput.readFloatArray();
            };

            void readPortable(MorphingPortableReader& dataInput, std::vector<double >& data) {
                data = dataInput.readDoubleArray();
            }

            /*****************************************************************************/
            void readPortable(BufferedDataInput& dataInput, byte& data) {
                data = dataInput.readByte();
            };

            void readPortable(BufferedDataInput& dataInput, bool& data) {
                data = dataInput.readBoolean();
            };

            void readPortable(BufferedDataInput& dataInput, char& data) {
                data = dataInput.readChar();
            };

            void readPortable(BufferedDataInput& dataInput, short & data) {
                data = dataInput.readShort();
            };

            void readPortable(BufferedDataInput& dataInput, int& data) {
                data = dataInput.readInt();
            };

            void readPortable(BufferedDataInput& dataInput, long & data) {
                data = dataInput.readLong();
            };

            void readPortable(BufferedDataInput& dataInput, float & data) {
                data = dataInput.readFloat();
            };

            void readPortable(BufferedDataInput& dataInput, double & data) {
                data = dataInput.readDouble();
            };

            void readPortable(BufferedDataInput& dataInput, std::string&  data) {
                data = dataInput.readUTF();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<byte>& data) {
                data = dataInput.readByteArray();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<char >& data) {
                data = dataInput.readCharArray();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<short >& data) {
                data = dataInput.readShortArray();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<int>& data) {
                data = dataInput.readIntArray();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<long >& data) {
                data = dataInput.readLongArray();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<float >& data) {
                data = dataInput.readFloatArray();
            };

            void readPortable(BufferedDataInput& dataInput, std::vector<double >& data) {
                data = dataInput.readDoubleArray();
            };
            /*****************************************************************************/

        }
    }
}