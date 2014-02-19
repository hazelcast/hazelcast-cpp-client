////
//// Created by sancar koyunlu on 9/6/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//#include "executor/RunnableTask.h"
//#include "hazelcast/client/serialization/PortableWriter.h"
//#include "hazelcast/client/serialization/PortableReader.h"
//
//
//RunnableTask::RunnableTask() {
//
//}
//
//RunnableTask::RunnableTask(std::string& name)
//:name(name) {
//
//}
//
//int RunnableTask::getFactoryId() const {
//    return 666;
//}
//
//int RunnableTask::getClassId() const {
//    return 1;
//}
//
//void RunnableTask::writePortable(serialization::PortableWriter& writer) const {
//    writer.writeUTF("n", name);
//}
//
//void RunnableTask::readPortable(serialization::PortableReader& reader) {
//    name = reader.readUTF("n");
//}
