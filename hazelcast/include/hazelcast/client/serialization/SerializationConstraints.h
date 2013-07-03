#ifndef HAZELCAST_SERIALIZATION_CONSTRAINTS
#define HAZELCAST_SERIALIZATION_CONSTRAINTS

#include "BufferedDataInput.h"
#include "BufferedDataOutput.h"
#include <vector>
#include <string>

typedef unsigned char byte;

namespace hazelcast {
    namespace client {
        namespace serialization {
            class EmptyPortableWriter {
            public:
                void writeInt(const char *fieldName, int value) {
                };

                void writeLong(const char *fieldName, long value) {
                };

                void writeBoolean(const char *fieldName, bool value) {
                };

                void writeByte(const char *fieldName, byte value) {
                };

                void writeChar(const char *fieldName, int value) {
                };

                void writeDouble(const char *fieldName, double value) {
                };

                void writeFloat(const char *fieldName, float value) {
                };

                void writeShort(const char *fieldName, short value) {
                };

                void writeUTF(const char *fieldName, const std::string& str) {
                };

                void writeNullPortable(const char *fieldName, int factoryId, int classId) {
                };

                void writeByteArray(const char *fieldName, const std::vector<byte>& x) {
                };

                void writeCharArray(const char *fieldName, const std::vector<char >&  data) {
                };

                void writeShortArray(const char *fieldName, const std::vector<short >&  data) {
                };

                void writeIntArray(const char *fieldName, const std::vector<int>&  data) {
                };

                void writeLongArray(const char *fieldName, const std::vector<long >&  data) {
                };

                void writeFloatArray(const char *fieldName, const std::vector<float >&  data) {
                };

                void writeDoubleArray(const char *fieldName, const std::vector<double >&  data) {
                };

                template <typename T>
                void writePortable(const char *fieldName, const T& portable) {
                };

                template <typename T>
                void writePortableArray(const char *fieldName, const std::vector<T>& values) {
                };

                BufferedDataOutput *getRawDataOutput() {
                };

            };

            class EmptyPortableReader {
            public:
                int readInt(const char *fieldName) {
                };

                long readLong(const char *fieldName) {
                };

                bool readBoolean(const char *fieldName) {
                };

                byte readByte(const char *fieldName) {
                };

                char readChar(const char *fieldName) {
                };

                double readDouble(const char *fieldName) {

                };

                float readFloat(const char *fieldName) {
                };

                short readShort(const char *fieldName) {
                };

                std::string readUTF(const char *fieldName) {
                };

                std::vector<byte> readByteArray(const char *fieldName) {
                };

                std::vector<char> readCharArray(const char *fieldName) {
                };

                std::vector<int> readIntArray(const char *fieldName) {
                };

                std::vector<long> readLongArray(const char *fieldName) {
                };

                std::vector<double> readDoubleArray(const char *fieldName) {
                };

                std::vector<float> readFloatArray(const char *fieldName) {
                };

                std::vector<short> readShortArray(const char *fieldName) {
                };

                template<typename T>
                T readPortable(const char *fieldName) {
                };

                template<typename T>
                std::vector< T > readPortableArray(const char *fieldName) {
                };

                BufferedDataInput *getRawDataInput() {

                };
            };

            template<class T> struct Is_Portable {
                static void constraints(T& t) {
                    int s = t.getFactoryId();
                    s = t.getClassId();
                    static EmptyPortableWriter ew;
                    static EmptyPortableReader er;
                    t.writePortable(ew);
                    t.readPortable(er);

                }

                static void const_constraints(const T& t) {
                    static EmptyPortableWriter ew;
                    t.writePortable(ew);
                    int s = t.getFactoryId();
                    s = t.getClassId();
                }

                Is_Portable() {
                    void(*p)(T&) = constraints;
                    void(*cp)(const T&) = const_constraints;
                }
            };

            template<class T> struct Is_DataSerializable {
                static void constraints(T& t) {
                    int s = t.getFactoryId();
                    s = t.getClassId();
                    BufferedDataOutput o;
                    std::vector<byte> a;
                    BufferedDataInput i(a);
                    t.writeData(o);
                    t.readData(i);
                }

                static void const_constraints(T& t) {
                    int s = t.getFactoryId();
                    s = t.getClassId();
                    BufferedDataOutput o;
                    t.writeData(o);
                }

                Is_DataSerializable() {
                    void(*p)(T&) = constraints;
                    void(*cp)(T&) = const_constraints;
                }
            };

        }
    }
}

#endif