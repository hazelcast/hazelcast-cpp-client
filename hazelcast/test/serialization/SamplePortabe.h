//
// Created by sancar koyunlu on 8/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_SamplePortabe
#define HAZELCAST_SamplePortabe

#include "Portable.h"
#include "PortableWriter.h"
#include "PortableReader.h"

using namespace hazelcast::client;

class Person {

};

class SamplePortable : public Portable {
public:
    int getFactoryId() const {
        return 4;
    }

    int getClassId() const {
        return 23;
    }

    void writePortable(serialization::PortableWriter & writer) const {
        writer.writePortable("s1", person);
        writer.writePortableArray("s2", personArray);
    }

    void readPortable(serialization::PortableReader & reader) {
        person = reader.readPortable("s1");
        personArray = reader.readPortableArray("s2");
    }
private:
    std::auto_ptr<Person> person;
    std::vector<std::auto_ptr<Person> > illegalArray;
    std::auto_ptr< std::vector<Person> > personArray;
};


#endif //HAZELCAST_SamplePortabe

