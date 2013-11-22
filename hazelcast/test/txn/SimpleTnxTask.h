//
// Created by Danny on 11/21/13.
//



#ifndef __SimpleTnxTask_H_
#define __SimpleTnxTask_H_

#include "hazelcast/client/TransactionalTaskContext.h"


using hazelcast::client::TransactionalTaskContext;

class SimpleTnxTask {

public:
    static const string mapName;
    static const string key;
    static const string expected;
    static const int expectedSZ;

    bool execute(TransactionalTaskContext &) const;
};

class SimpleTnxTaskFail {

public:
    static const string mapName;
    static const string key;
    static const int expectedSZ;

    bool execute(TransactionalTaskContext &) const;
};


#endif //__SimpleTnxTask_H_
