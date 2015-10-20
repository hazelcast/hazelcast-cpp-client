//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION_WRITER
#define HAZELCAST_CLASS_DEFINITION_WRITER

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/serialization/FieldType.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/serialization/pimpl/PortableVersionHelper.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;

            namespace pimpl {

                class PortableContext;

                class HAZELCAST_API ClassDefinitionWriter {
                public:

                    ClassDefinitionWriter(PortableContext& portableContext, ClassDefinitionBuilder& builder);

                    void writeInt(const char *fieldName, int value);

                    void writeLong(const char *fieldName, long value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, short value);

                    void writeUTF(const char *fieldName, const std::string *str);

                    void writeByteArray(const char *fieldName, const std::vector<byte> *values);

                    void writeCharArray(const char *fieldName, const std::vector<char> *data);

                    void writeShortArray(const char *fieldName, const std::vector<short> *data);

                    void writeIntArray(const char *fieldName, const std::vector<int> *data);

                    void writeLongArray(const char *fieldName, const std::vector<long> *data);

                    void writeFloatArray(const char *fieldName, const std::vector<float> *data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double> *data);

                    template<typename T>
                    void writeNullPortable(const char *fieldName) {

                        T portable;
                        int factoryId = portable.getFactoryId();
                        int classId = portable.getClassId();
                        boost::shared_ptr<ClassDefinition> nestedClassDef = context.lookupClassDefinition(factoryId, classId, context.getVersion());
                        if (nestedClassDef == NULL) {
                            throw exception::HazelcastSerializationException("ClassDefWriter::writeNullPortable",
                                     "Cannot write null portable without explicitly registering class definition!");
                        }
                        builder.addPortableField(fieldName, nestedClassDef);
                    }

                    template<typename T>
                    void writePortable(const char *fieldName, const T *portable) {
                        if (NULL == portable) {
                            throw exception::HazelcastSerializationException("ClassDefinitionWriter::writePortable",
                                     "Cannot write null portable without explicitly registering class definition!");
                        }

                        boost::shared_ptr<ClassDefinition> nestedClassDef = createNestedClassDef(*portable);
                        builder.addPortableField(fieldName, nestedClassDef);
                    };

                    template<typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T> *portables) {
                        if (NULL == portables || portables->size() == 0) {
                            throw exception::HazelcastSerializationException(
                                    "ClassDefinitionWriter::writePortableArray",
                                    "Cannot write null portable array without explicitly registering class definition!");
                        }
                        boost::shared_ptr<ClassDefinition> nestedClassDef = createNestedClassDef((*portables)[0]);
                        builder.addPortableArrayField(fieldName, nestedClassDef);
                    };

                    boost::shared_ptr<ClassDefinition> registerAndGet();

                    ObjectDataOutput& getRawDataOutput();

                    void end();

                private:
                    boost::shared_ptr<ClassDefinition> createNestedClassDef(const Portable& portable);

                    ObjectDataOutput emptyDataOutput;
                    ClassDefinitionBuilder& builder;
                    PortableContext& context;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */

