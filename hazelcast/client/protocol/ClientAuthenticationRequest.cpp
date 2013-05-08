//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ClientAuthenticationRequest.h"
#include "Credentials.h"

int ClientAuthenticationRequest::getFactoryId() {
    return -3;
}

int ClientAuthenticationRequest::getClassId() {
    return 2;
}

void ClientAuthenticationRequest::writePortable(hazelcast::client::serialization::PortableWriter & writer) {
    Credentials portable;
    writer.writePortable("credentials", portable);
    writer.writeNullPortable("principal", -3, 3);
    writer.writeBoolean("reAuth", true);

}

void ClientAuthenticationRequest::readPortable(hazelcast::client::serialization::PortableReader & reader) {
}
