//
// Created by Danny on 11/21/13.
//

#include <iostream>
#include "SimpleTnxTask.h"
#include "hazelcast/client/TransactionalMap.h"


using std::cout;
using std::exception;
using hazelcast::client::TransactionalMap;


const string SimpleTnxTask::mapName = "map1_simple_executeTransaction_test";
const string SimpleTnxTask::key = "key1";
const string SimpleTnxTask::expected = "val1";
const int SimpleTnxTask::expectedSZ = 1;

bool SimpleTnxTask::execute(TransactionalTaskContext &context) const {

    TransactionalMap<string, string> map = context.getMap<string, string>(mapName);
    map.put(key, expected);

    return true;
}



const string SimpleTnxTaskFail::mapName = "map1_simple_executeTransaction_test_exception";
const string SimpleTnxTaskFail::key = "key1";
const int SimpleTnxTaskFail::expectedSZ = 0;

bool SimpleTnxTaskFail::execute(TransactionalTaskContext &context) const {

    TransactionalMap<string, string> map = context.getMap<string, string>(mapName);
    map.put(key, "Val1");
    map.put("key2", "Val2");
    map.put("key3", "Val3");

    throw  exception();
}
