//
//  DefaultPortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "DefaultPortableWriter.h"

//DefaultPortableWriter::DefaultPortableWriter(PortableSerializer serializer, BufferObjectDataOutput out, ClassDefinitionImpl cd){};

DefaultPortableWriter::DefaultPortableWriter(TypeSerializer serializer, BufferObjectDataOutput out, ClassDefinitionImpl cd){};

void DefaultPortableWriter::writeInt(string fieldName, int value) throw(ios_base::failure){};

void DefaultPortableWriter::writeLong(string fieldName, long value) throw(ios_base::failure){};

void DefaultPortableWriter::writeUTF(string fieldName, string str) throw(ios_base::failure){};

void DefaultPortableWriter::writeBoolean(string fieldName, bool value) throw(ios_base::failure){};

void DefaultPortableWriter::writeByte(string fieldName, byte value) throw(ios_base::failure){};

void DefaultPortableWriter::writeChar(string fieldName, int value) throw(ios_base::failure){};

void DefaultPortableWriter::writeDouble(string fieldName, double value) throw(ios_base::failure){};

void DefaultPortableWriter::writeFloat(string fieldName, float value) throw(ios_base::failure){};

void DefaultPortableWriter::writeShort(string fieldName, short value) throw(ios_base::failure){};

//void DefaultPortableWriter::writePortable(string fieldName, Portable portable) throw(ios_base::failure){};

void DefaultPortableWriter::writeByteArray(string fieldName, byte* values) throw(ios_base::failure){};

void DefaultPortableWriter::writeCharArray(string fieldName, char* values) throw(ios_base::failure){};

void DefaultPortableWriter::writeIntArray(string fieldName, int* values) throw(ios_base::failure){};

void DefaultPortableWriter::writeLongArray(string fieldName, long* values) throw(ios_base::failure){};

void DefaultPortableWriter::writeDoubleArray(string fieldName, double* values) throw(ios_base::failure){};

void DefaultPortableWriter::writeFloatArray(string fieldName, float* values) throw(ios_base::failure){};

void DefaultPortableWriter::writeShortArray(string fieldName, short* values) throw(ios_base::failure){};

void DefaultPortableWriter::writePortableArray(string fieldName, Portable* portables) throw(ios_base::failure){};

void DefaultPortableWriter::setPosition(string fieldName) throw(ios_base::failure){};

void DefaultPortableWriter::writeNullablestring(DataOutput out, string obj){};
