//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TestSimpleXML
#define HAZELCAST_TestSimpleXML

#include <string>

class TestCustomXSerializable {
public:
    int id;

    int getSerializerId() const {
        return 666;
    };
};

class TestCustomPerson {
public:

    std::string getName() const {
        return name;
    };

    void setName(const std::string & param) {
        name = param;
    };

    int getSerializerId() const {
        return 999;
    };

private:
    std::string name;
};

#endif //HAZELCAST_TestSimpleXML

