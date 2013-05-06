//
// Created by sancar koyunlu on 5/6/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef __SimpleDataSerializable_H_
#define __SimpleDataSerializable_H_

#include <iostream>
#include "DataSerializable.h"
#include "DataOutput.h"
#include "DataInput.h"

using namespace hazelcast::client::serialization;

class SimpleDataSerializable : public DataSerializable {
public:

    SimpleDataSerializable() {

    }

    SimpleDataSerializable(std::vector<byte> data) {
        this->data = data;
    }

    void writeData(DataOutput & out) const {
        out.writeInt(data.size());
        out.write(data);
    }

    void readData(DataInput & in) {
        int len = in.readInt();
        data.resize(len, 0);
        in.readFully(data);
    }

    std::vector<byte> data;

};


#endif //__SimpleDataSerializable_H_
