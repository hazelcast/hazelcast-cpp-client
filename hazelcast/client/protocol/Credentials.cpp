//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Credentials.h"

int Credentials::getFactoryId() {
    return -1;
}

int Credentials::getClassId() {
    return 1;
}

void Credentials::writePortable(hazelcast::client::serialization::PortableWriter & writer) {
    writer.writeUTF("principal", "dev");
    writer.writeUTF("endpoint", "");
    char const *pasw = "dev-pass";
    vector<byte> pwd(pasw, pasw + 8);
    writer.writeByteArray("pwd", pwd);
}

void Credentials::readPortable(hazelcast::client::serialization::PortableReader & reader) {

}
