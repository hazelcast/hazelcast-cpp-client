//
// Created by sancar koyunlu on 7/1/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TestSimpleXML
#define HAZELCAST_TestSimpleXML

#include <string>

class TestCustomXSerializable {
public:
    std::string name;

    virtual int getSerializerId() const{
        return 666;
    };
};

class TestCustomPerson {
public:

    virtual std::string getName() const = 0;

    virtual void setName(const std::string&) = 0;

    virtual int getSerializerId() const {
        return 999;
    };
};

class TestCustomStudent : public TestCustomPerson {
public:

    int id;
    std::string name;

    std::string getName() const {
        return name;
    };

    void setName(const std::string & param) {
        name = param;
    };
};

class TestCustomTwoNamedPerson : public TestCustomPerson {
public:

    std::string surname;
    std::string name;

    std::string getName() const {
        return name;
    };

    void setName(const std::string & param) {
        name = param;
    };
};


#endif //HAZELCAST_TestSimpleXML

