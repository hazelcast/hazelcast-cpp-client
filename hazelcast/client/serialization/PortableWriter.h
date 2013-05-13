////
////  PortableWriter.h
////  Server
////
////  Created by sancar koyunlu on 1/10/13.
////  Copyright (c) 2013 sancar koyunlu. All rights reserved.
////
//
//#ifndef HAZELCAST_PORTABLE_WRITER
//#define HAZELCAST_PORTABLE_WRITER
//
//#include "DataOutput.h"
//#include "PortableSerializer.h"
//#include "FieldDefinition.h"
//#include "HazelcastException.h"
//#include "FieldType.h"
//#include <iostream>
//#include <string>
//#include <set>
//
//using namespace std;
//
//namespace hazelcast {
//    namespace client {
//        namespace serialization {
//
//            class ClassDefinition;
//
//            class DataInput;
//
//            class Portable;
//
//            class PortableWriter {
//            public:
//
//                enum Type {
//                    DEFAULT, CLASS_DEFINITION_WRITER
//                };
//
//                PortableWriter(PortableSerializer *serializer, boost::shared_ptr<ClassDefinition> cd, DataOutput *output, Type id);
//
//                void writeInt(string fieldName, int value);
//
//                void writeLong(string fieldName, long value);
//
//                void writeBoolean(string fieldName, bool value);
//
//                void writeByte(string fieldName, byte value);
//
//                void writeChar(string fieldName, int value);
//
//                void writeDouble(string fieldName, double value);
//
//                void writeFloat(string fieldName, float value);
//
//                void writeShort(string fieldName, short value);
//
//                void writeUTF(string fieldName, string str);
//
//                void writePortable(string fieldName, Portable& portable);
//
//                void writeNullPortable(string fieldName, int factoryId, int classId);
//
//                void writeByteArray(string fieldName, std::vector<byte>&);
//
//                void writeCharArray(string fieldName, std::vector<char>&);
//
//                void writeIntArray(string fieldName, std::vector<int>&);
//
//                void writeLongArray(string fieldName, std::vector<long>&);
//
//                void writeDoubleArray(string fieldName, std::vector<double>&);
//
//                void writeFloatArray(string fieldName, std::vector<float>&);
//
//                void writeShortArray(string fieldName, std::vector<short>&);
//
//                template <typename T>
//                void writePortableArray(string fieldName, std::vector<T>& values) {
//                    if (checkType(fieldName, values)) return;
//                    setPosition(fieldName);
//                    int len = values.size();
//                    output->writeInt(len);
//                    if (len > 0) {
//                        int offset = output->position();
//                        output->position(offset + len * sizeof (int));
//                        for (int i = 0; i < len; i++) {
//                            output->writeInt(offset + i * sizeof (int), output->position());
//                            serializer->write(*output, values[i]);
//                        }
//                    }
//                };
//
//                DataOutput *const getRawDataOutput();
//
//            private:
//
//                void setPosition(string fieldName);
//
//                bool checkType(string fieldName, FieldType const& id);
//
//                bool checkType(string fieldName, int factoryId , int classId);
//
//                bool checkType(string fieldName, Portable& portable);
//
//                template<typename T>
//                bool checkType(string fieldName, std::vector<T>& portables) {
//                    if (id == CLASS_DEFINITION_WRITER) {
//                        if (!raw) {
//                            int classId = portables[0].getClassId();
//                            int factoryId = portables[0].getFactoryId();
//                            for (int i = 1; i < portables.size(); i++) {
//                                if (portables[i].getClassId() != classId) {
//                                    throw hazelcast::client::HazelcastException("Illegal Argument Exception");
//                                }
//                            }
//                            FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY, factoryId, classId);
//                            addNestedField(portables[0], fd);
//                        }
//                        return true;
//                    } else {
//                        return false;
//                    }
//
//
//                };
//
//                void addNestedField(Portable& p, FieldDefinition& fd);
//
//                Type id;
//                int index;
//                bool raw;
//                PortableSerializer *serializer;
//                DataOutput *output;
//                int offset;
//                std::set<std::string> writtenFields;
//                boost::shared_ptr<ClassDefinition> cd;
//
//            };
//
//        }
//    }
//}
//#endif /* HAZELCAST_PORTABLE_WRITER */
