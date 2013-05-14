////
//// Created by sancar koyunlu on 5/6/13.
//// Copyright (c) 2013 sancar koyunlu. All rights reserved.
////
//// To change the template use AppCode | Preferences | File Templates.
////
//
//
//
//#ifndef __RawDataPortable_H_
//#define __RawDataPortable_H_
//
//#include <iostream>
//#include "TestNamedPortable.h"
//#include "SimpleDataSerializable.h"
//
//
//class TestRawDataPortable : public Portable {
//public:
//
//    long l;
//    std::vector<char> c;
//    TestNamedPortable p;
//    int k;
//    std::string s;
//    SimpleDataSerializable sds;
//
//    TestRawDataPortable() {
//
//    }
//
//    TestRawDataPortable(long l, std::vector<char> c, TestNamedPortable p, int k, std::string s, SimpleDataSerializable sds) {
//        this->l = l;
//        this->c = c;
//        this->p = p;
//        this->k = k;
//        this->s = s;
//        this->sds = sds;
//    }
//
//
//    int getFactoryId() {
//        return 1;
//    };
//
//    int getClassId() {
//        return 4;
//    };
//
//    void writePortable(PortableWriter & writer) {
//        writer.writeLong("l", l);
//        writer.writeCharArray("c", c);
//        writer.writePortable("p", p);
//        BufferedDataOutput *const output = writer.getRawDataOutput();
//        output->writeInt(k);
//        output->writeUTF(s);
////        output->writeObject(sds);
//    };
//
//    void readPortable(PortableReader & reader) {
//        l = reader.readLong("l");
//        c = reader.readCharArray("c");
//        p = reader.readPortable<TestNamedPortable>("p");
//        BufferedDataInput *input = reader.getRawDataInput();
//        k = input->readInt();
//        s = input->readUTF();
////        sds = input->readObject();
//    };
//
//    bool operator ==(TestRawDataPortable& m) {
//        if (this == &m)
//            return true;
//        if (l != m.l) return false;
//        if (c != m.c) return false;
//        if (p != m.p) return false;
//        if (k != m.k) return false;
//        if (s.compare(m.s) != 0) return false;
////        if (sds != m.sds) return false;
//        return true;
//    };
//
//    bool operator !=(TestRawDataPortable& m) {
//        return !(*this == m);
//    };
//};
//
//
//#endif //__RawDataPortable_H_
//
