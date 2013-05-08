//
// Created by sancar koyunlu on 5/8/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "CustomSerializer.h"
#include "CustomClass.h"

int CustomSerializer::getTypeId() {
    return 5;
}

void CustomSerializer::write(DataOutput *out, void *object) {
    std::cout << "writingtoOut" << std::endl;
    CustomClass *customClass = reinterpret_cast<CustomClass*>(object);
    out->writeInt(customClass->a);
}

void *CustomSerializer::read(DataInput& in) {
    std::cout << "readingFromIn" << std::endl;
    int x = in.readInt();
    return new CustomClass(x);
}
