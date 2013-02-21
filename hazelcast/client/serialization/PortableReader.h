//
//  PortableReader.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE_READER
#define HAZELCAST_PORTABLE_READER

#include "ClassDefinition.h"
#include "DataInput.h"
#include "FieldDefinition.h"
#include "PortableSerializer.h"

#include <iostream>
#include <string>
#include <memory>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class Portable;
            class BufferObjectDataInput;

            typedef unsigned char byte;

            class PortableReader {
            public:

                enum Type {
                    DEFAULT, MORPHING
                };

                PortableReader(PortableSerializer *serializer, DataInput& input, boost::shared_ptr<ClassDefinition> cd, Type isMorphing);

                int readInt(string fieldName);

                long readLong(string fieldName);

                bool readBoolean(string fieldName);

                byte readByte(string fieldName);

                char readChar(string fieldName);

                double readDouble(string fieldName);

                float readFloat(string fieldName);

                short readShort(string fieldName);

                string readUTF(string fieldName);

                std::vector<byte> readByteArray(string fieldName);

                std::vector<char> readCharArray(string fieldName);

                std::vector<int> readIntArray(string fieldName);

                std::vector<long> readLongArray(string fieldName);

                std::vector<double> readDoubleArray(string fieldName);

                std::vector<float> readFloatArray(string fieldName);

                std::vector<short> readShortArray(string fieldName);

                template<typename T>
                T readPortable(string fieldName) {
                    if (type == MORPHING && !isFieldMorphed) return morphPortable<T >(fieldName);
                    if (!cd->isFieldDefinitionExists(fieldName))
                        throw "throwUnknownFieldException" + fieldName;

                    FieldDefinition fd = cd->get(fieldName);

                    int pos = getPosition(&fd);
                    input->position(pos);
                    bool isNull = input->readBoolean();
                    if (isNull) {
                        return T();
                    }
                    input->setDataClassId(fd.getClassId());
                    std::auto_ptr<Portable> p(serializer->read(*input));

                    input->setDataClassId(cd->getClassId());
                    T portable = *dynamic_cast<T *> (p.get());
                    return portable;
                };

                template<typename T>
                std::vector< T > readPortableArray(string fieldName) {
                    if (type == MORPHING && !isFieldMorphed) return morphPortableArray<T >(fieldName);
                    if (!cd->isFieldDefinitionExists(fieldName))
                        throw "throwUnknownFieldException" + fieldName;
                    FieldDefinition fd = cd->get(fieldName);
                    int currentPos = input->position();
                    int pos = getPosition(fieldName);
                    input->position(pos);
                    int len = input->readInt();
                    std::vector< T > portables(len);
                    if (len > 0) {
                        int offset = input->position();
                        input->setDataClassId(fd.getClassId());
                        int start;
                        for (int i = 0; i < len; i++) {
                            start = input->readInt(offset + i * sizeof (int));
                            input->position(start);
                            portables[i] = *dynamic_cast<T *> (serializer->read(*input).get());
                        }
                    }
                    input->position(currentPos);
                    return portables;
                };

                int morphInt(string fieldName);

                long morphLong(string fieldName);

                bool morphBoolean(string fieldName);

                byte morphByte(string fieldName);

                char morphChar(string fieldName);

                double morphDouble(string fieldName);

                float morphFloat(string fieldName);

                short morphShort(string fieldName);

                string morphUTF(string fieldName);

                std::vector<byte> morphByteArray(string fieldName);

                std::vector<char> morphCharArray(string fieldName);

                std::vector<int> morphIntArray(string fieldName);

                std::vector<long> morphLongArray(string fieldName);

                std::vector<double> morphDoubleArray(string fieldName);

                std::vector<float> morphFloatArray(string fieldName);

                std::vector<short> morphShortArray(string fieldName);

                template <typename T>
                T morphPortable(string fieldName) {
                    isFieldMorphed = true;
                    if (!cd->isFieldDefinitionExists(fieldName))
                        throw "throwUnknownFieldException" + fieldName;
                    FieldDefinition fd = cd->get(fieldName);

                    if (fd.getType() != FieldDefinition::TYPE_PORTABLE) {
                        throw "IncompatibleClassChangeError";
                    }
                    return readPortable<T >(fieldName);
                };

                template <typename T>
                std::vector< T > morphPortableArray(string fieldName) {
                    isFieldMorphed = true;
                    if (!cd->isFieldDefinitionExists(fieldName))
                        throw "throwUnknownFieldException" + fieldName;
                    FieldDefinition fd = cd->get(fieldName);

                    if (fd.getType() != FieldDefinition::TYPE_PORTABLE_ARRAY) {
                        throw "IncompatibleClassChangeError";
                    }
                    return readPortableArray<T >(fieldName);
                };


            protected:

                int getPosition(string fieldName);

                int getPosition(FieldDefinition *);

                PortableSerializer *serializer;
                boost::shared_ptr<ClassDefinition> cd;
                DataInput *input;
                int offset;
                Type type;
                bool isFieldMorphed;
            };

        }
    }
}
#endif /* HAZELCAST_PORTABLE_READER */
