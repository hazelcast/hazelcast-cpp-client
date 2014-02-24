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
#include "hazelcast/client/serialization/pimpl/FieldType.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinition.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/pimpl/FieldDefinition.h"
#include <string>


namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;
            namespace pimpl {

                class SerializationContext;

                class HAZELCAST_API ClassDefinitionWriter {
                public:

                    ClassDefinitionWriter(int factoryId, int classId, int version, SerializationContext &serializationContext);

                    void writeInt(const char *fieldName, int value);

                    void writeLong(const char *fieldName, long value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, short value);

                    void writeUTF(const char *fieldName, const std::string &str);

                    void writeNullPortable(const char *fieldName, int factoryId, int classId);

                    void writeByteArray(const char *fieldName, const std::vector<byte> &values);

                    void writeCharArray(const char *fieldName, const std::vector<char > &data);

                    void writeShortArray(const char *fieldName, const std::vector<short > &data);

                    void writeIntArray(const char *fieldName, const std::vector<int> &data);

                    void writeLongArray(const char *fieldName, const std::vector<long > &data);

                    void writeFloatArray(const char *fieldName, const std::vector<float > &data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double > &data);

                    template <typename T>
                    void writePortable(const char *fieldName, const T &portable) {
                        FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE, portable.getFactoryId(), portable.getClassId());
                        addNestedField(portable, fd);
                    };

                    template <typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T> &portables) {
                        int classId = portables[0].getClassId();
                        int factoryId = portables[0].getFactoryId();
                        FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY, factoryId, classId);
                        addNestedField(portables[0], fd);
                    };

                    boost::shared_ptr<ClassDefinition> getClassDefinition();

                    ObjectDataOutput &getRawDataOutput();

                    boost::shared_ptr<ClassDefinition> getOrBuildClassDefinition(const Portable &p);

                    void end();

                private:
                    void addField(const char *fieldName, FieldType const &);

                    template <typename T>
                    void addNestedField(T &p, FieldDefinition &fd) {
                        cd->add(fd);
                        boost::shared_ptr<ClassDefinition> nestedCd = getOrBuildClassDefinition(p);
                        cd->add(nestedCd);
                    };

                    int factoryId;
                    int classId;
                    int index;
                    bool raw;
                    ObjectDataOutput emptyDataOutput;
                    boost::shared_ptr<ClassDefinition> cd;
                    SerializationContext &context;

                };
            }
        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION_WRITER */
