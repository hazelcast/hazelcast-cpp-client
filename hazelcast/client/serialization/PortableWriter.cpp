//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableWriter.h"
#include "DataOutput.h"
#include "ClassDefinition.h"
#include "PortableSerializer.h"
#include "FieldDefinition.h"

namespace hazelcast {
namespace client {
namespace serialization {

PortableWriter::PortableWriter(){

};

PortableWriter::PortableWriter(PortableSerializer* serializer, DataOutput* output, ClassDefinition* cd) {
    this->serializer = serializer;
    this->output = output;
    this->offset = output->position();
    this->cd = cd;
    char zeros[ cd->getFieldCount() * sizeof (int) ];
    this->output->write(zeros, 0, cd->getFieldCount() * sizeof (int));

};

void PortableWriter::writeInt(string fieldName, int value) {
    setPosition(fieldName);
    output->writeInt(value);
};

void PortableWriter::writeLong(string fieldName, long value) {
    setPosition(fieldName);
    output->writeLong(value);
};

void PortableWriter::writeBoolean(string fieldName, bool value) {
    setPosition(fieldName);
    output->writeBoolean(value);
};

void PortableWriter::writeByte(string fieldName, byte value) {
    setPosition(fieldName);
    output->writeByte(value);
};

void PortableWriter::writeChar(string fieldName, int value) {
    setPosition(fieldName);
    output->writeChar(value);
};

void PortableWriter::writeDouble(string fieldName, double value) {
    setPosition(fieldName);
    output->writeDouble(value);
};

void PortableWriter::writeFloat(string fieldName, float value) {
    setPosition(fieldName);
    output->writeFloat(value);
};

void PortableWriter::writeShort(string fieldName, short value) {
    setPosition(fieldName);
    output->writeShort(value);
};

void PortableWriter::writeUTF(string fieldName, string str) {
    setPosition(fieldName);
    output->writeUTF(str);
};

void PortableWriter::writePortable(string fieldName, Portable& portable) {
    setPosition(fieldName);
    bool isNull = &portable == NULL;
    output->writeBoolean(isNull);
    if (!isNull) {
        serializer->write(output, portable);
    }
};

void PortableWriter::writeByteArray(string fieldName, std::vector<byte>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeByte(values[i]);
        }
    }
};

void PortableWriter::writeCharArray(string fieldName, std::vector<char>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeChar(values[i]);
        }
    }
};

void PortableWriter::writeIntArray(string fieldName, std::vector<int>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeInt(values[i]);
        }
    }
};

void PortableWriter::writeLongArray(string fieldName, std::vector<long>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeLong(values[i]);
        }
    }
};

void PortableWriter::writeDoubleArray(string fieldName, std::vector<double>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeDouble(values[i]);
        }
    }
};

void PortableWriter::writeFloatArray(string fieldName, std::vector<float>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeFloat(values[i]);
        }
    }
};

void PortableWriter::writeShortArray(string fieldName, std::vector<short>& values) {
    setPosition(fieldName);
    int len = values.size();
    output->writeInt(len);
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            output->writeShort(values[i]);
        }
    }
};

void PortableWriter::setPosition(string fieldName) {
    if (!cd->isFieldDefinitionExists(fieldName)) {
        std::string error;
        error += "HazelcastSerializationException( Invalid field name: '";
        error += fieldName;
        error += "' for ClassDefinition {id: ";
        error += cd->getClassId();
        error += ", version: ";
        error += cd->getVersion();
        error += "}";

        throw error;
    }
    FieldDefinition fd = cd->get(fieldName);

    int pos = output->position();
    int index = fd.getIndex();
    // index = fieldIndex++; // if class versions are the same.
    output->writeInt(offset + index * sizeof (int), pos);

};

}}}