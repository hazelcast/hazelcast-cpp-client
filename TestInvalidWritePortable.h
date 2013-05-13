////
//// Created by sancar koyunlu on 5/6/13.
//// Copyright (c) 2013 sancar koyunlu. All rights reserved.
////
//// To change the template use AppCode | Preferences | File Templates.
////
//
//
//
//#ifndef __TestInvalidWritePortable_H_
//#define __TestInvalidWritePortable_H_
//
//#include <iostream>
//#include "Portable.h"
//#include "PortableWriter.h"
//#include "PortableReader.h"
//
//using namespace hazelcast::client::serialization;
//
//class TestInvalidWritePortable : public Portable {
//public:
//
//    TestInvalidWritePortable() {
//
//    }
//
//    TestInvalidWritePortable(long l, int i, std::string s) {
//        this->l = l;
//        this->i = i;
//        this->s = l;
//    }
//
//
//    int getFactoryId() {
//        return 1;
//    }
//
//    int getClassId() {
//        return 5;
//    }
//
//    void writePortable(PortableWriter & writer) {
//        writer.writeLong("l", l);
//        DataOutput *output = writer.getRawDataOutput();
//        writer.writeInt("i", i);
//        output->writeUTF(s);
//    }
//
//    void readPortable(PortableReader & reader) {
//        l = reader.readLong("l");
//        i = reader.readInt("i");
//        s = reader.readUTF("s");
//    }
//
//    long l;
//    int i;
//    std::string s;
//};
//
//
//#endif //__TestInvalidWritePortable_H_
//
//
