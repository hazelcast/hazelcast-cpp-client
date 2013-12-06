//
// Created by sancar koyunlu on 9/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_RunnableTask
#define HAZELCAST_RunnableTask

#include "hazelcast/client/Portable.h"
#include <string>

using namespace hazelcast::client;

class HAZELCAST_API RunnableTask : public Portable {
public:

    RunnableTask();

    RunnableTask(std::string& name);

    int getFactoryId() const;

    int getClassId() const;

    void writePortable(serialization::PortableWriter& writer) const;

    void readPortable(serialization::PortableReader& reader);

private:
    std::string name;
};


#endif //HAZELCAST_RunnableTask
