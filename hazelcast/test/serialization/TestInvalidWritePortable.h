//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __TestInvalidWritePortable_H_
#define __TestInvalidWritePortable_H_

#include <iostream>


using namespace hazelcast::client::serialization;

class TestInvalidWritePortable : public Portable {
public:

    TestInvalidWritePortable() {

    }

    TestInvalidWritePortable(long l, int i, std::string s) {
        this->l = l;
        this->i = i;
        this->s = l;
    }

    inline int getFactoryId() const {
        return 1;
    }

    inline int getClassId() const {
        return 5;
    }

    template<typename HzWriter>
    inline void writePortable(HzWriter& writer) const {
        writer.writeLong("l", l);
        serialization::ObjectDataOutput *out = writer.getRawDataOutput();
        out->writeInt(i);
        writer.writeUTF("s", s);
    };

    template<typename HzReader>
    inline void readPortable(HzReader& reader) {
        l = reader.readLong("l");
        i = reader.readInt("i");
        s = reader.readLong("s");
    };

    long l;
    int i;
    std::string s;
};


#endif //__TestInvalidWritePortable_H_


