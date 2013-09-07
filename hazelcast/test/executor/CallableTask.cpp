//
// Created by sancar koyunlu on 9/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CallableTask.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"


CallableTask::CallableTask() {

}

CallableTask::CallableTask(std::string& name)
:name(name) {

}

int CallableTask::getFactoryId() const {
    return 666;
}

int CallableTask::getClassId() const {
    return 2;
}

void CallableTask::writeData(serialization::ObjectDataOutput& writer) const {
    writer.writeUTF(name);
}

void CallableTask::readData(serialization::ObjectDataInput& reader) {
    name = reader.readUTF();
}
