//
// Created by sancar koyunlu on 9/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "FailingTask.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"

FailingTask::FailingTask() {

}

int FailingTask::getFactoryId() const {
    return 666;
}

int FailingTask::getClassId() const {
    return 1;
}

void FailingTask::writeData(serialization::ObjectDataOutput& writer) const {

}

void FailingTask::readData(serialization::ObjectDataInput& reader) {

}
