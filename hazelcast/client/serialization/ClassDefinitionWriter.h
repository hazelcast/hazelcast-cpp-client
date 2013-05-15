//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION_WRITER
#define HAZELCAST_CLASS_DEFINITION_WRITER

#include "FieldDefinition.h"
#include "HazelcastException.h"
#include "FieldType.h"
#include "ConstantSerializers.h"
#include "ClassDefinition.h"
#include "SerializationContext.h"
#include <iosfwd>
#include <string>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinitionWriter {
            public:

                ClassDefinitionWriter(int factoryId, int classId, int version, SerializationContext *serializationContext);

                ClassDefinitionWriter& operator [](std::string fieldName);

                void writeInt(int value);

                void writeLong(long value);

                void writeBoolean(bool value);

                void writeByte(byte value);

                void writeChar(int value);

                void writeDouble(double value);

                void writeFloat(float value);

                void writeShort(short value);

                void writeUTF(string str);

                void writeNullPortable(int factoryId, int classId);

                void writeByteArray(std::vector<byte>&);

                void writeCharArray(std::vector<char>&);

                void writeIntArray(std::vector<int>&);

                void writeLongArray(std::vector<long>&);

                void writeDoubleArray(std::vector<double>&);

                void writeFloatArray(std::vector<float>&);

                void writeShortArray(std::vector<short>&);


                template <typename T>
                void writePortable(T& portable) {
                    if (!raw) {
                        FieldDefinition fd = FieldDefinition(index++, lastFieldName, FieldTypes::TYPE_PORTABLE, getFactoryId(portable), getClassId(portable));
                        addNestedField(portable, fd);
                    }
                };

                template <typename T>  //TODO duplicate code because of cyclic dependency look : PortableSerializer
                void writePortableArray(std::vector<T>& portables) {
                    if (!raw) {
                        int classId = portables[0].getClassId();
                        int factoryId = portables[0].getFactoryId();
                        for (int i = 1; i < portables.size(); i++) {
                            if (portables[i].getClassId() != classId) {
                                throw hazelcast::client::HazelcastException("Illegal Argument Exception");
                            }
                        }
                        FieldDefinition fd(index++, lastFieldName, FieldTypes::TYPE_PORTABLE_ARRAY, factoryId, classId);
                        addNestedField(portables[0], fd);
                    }
                };

                boost::shared_ptr<ClassDefinition> getClassDefinition();

            private:
                void addField(FieldType const&);

                template <typename T>
                void addNestedField(T& p, FieldDefinition& fd) {
                    cd->add(fd);
                    boost::shared_ptr<ClassDefinition> nestedCd = getOrBuildClassDefinition(p);
                    cd->add(nestedCd);
                };

                template <typename T>
                boost::shared_ptr<ClassDefinition> getOrBuildClassDefinition(T& p) {
                    boost::shared_ptr<ClassDefinition> cd;

                    int factoryId = getFactoryId(p);
                    int classId = getClassId(p);
                    if (context->isClassDefinitionExists(factoryId, classId)) {
                        cd = context->lookup(factoryId, classId);
                    } else {
                        ClassDefinitionWriter classDefinitionWriter(factoryId, classId, context->getVersion(), context);
                        writePortable(p);
                        cd = classDefinitionWriter.getClassDefinition();
                        context->registerClassDefinition(cd);
                    }

                    return cd;
                };

                int factoryId;
                int classId;
                int index;
                bool raw;
                bool writingPortable;
                std::string lastFieldName; //TODO needs more thoughts on name
                boost::shared_ptr<ClassDefinition> cd;
                SerializationContext *context;

            };

            template<typename T>
            inline void operator <<(ClassDefinitionWriter& classDefinitionWriter, T data) {
                //TODO i probably need to add more here
                //........
                writePortable(classDefinitionWriter, data);
//                classDefinitionWriter.writePortable(data);
            };
        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */
