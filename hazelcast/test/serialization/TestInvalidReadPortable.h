//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __TestInvalidReadPortable_H_
#define __TestInvalidReadPortable_H_

#include <iostream>


using namespace hazelcast::client::serialization;

class TestInvalidReadPortable : public Portable{
public:

    TestInvalidReadPortable() {

    }

    inline int getFactoryId() const{
        return 1;
    }

    inline int getClassId() const{
        return 6;
    }

    template<typename HzWriter>
    inline void writePortable(HzWriter& writer) const{
        writer.writeLong("l", l);
        writer.writeInt("i", i);
        writer.writeUTF("s", s);
    };

    template<typename HzReader>
    inline void readPortable(HzReader& reader) {
        l = reader.readLong("l");
        serialization::ObjectDataInput *in = reader.getRawDataInput();
        i = in->readInt();
        s = reader.readLong("s");
    };
    
    TestInvalidReadPortable(long l, int i, std::string s) {
        this->l = l;
        this->i = i;
        this->s = l;
    }

    long l;
    int i;
    std::string s;
};
#endif //__TestInvalidReadPortable_H_


