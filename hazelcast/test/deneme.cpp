//
// Created by sancar koyunlu on 7/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include <ostream>
#include <iostream>
#include "deneme.h"
#include "AtomicPointer.h"
#include "Member.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

void putMap(hazelcast::util::AtomicPointer<std::map<int, int>> *atomicPointer) {
    int t = 1;
    while (true) {
        t = t == 1 ? 2 : 1;
        std::map<int, int> *map1 = new std::map<int, int>;
        for (int i = 0; i < 20; i++) {
            (*map1)[i] = i * t;
        }
        atomicPointer->set(map1);
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));

    }
};

void printMap(hazelcast::util::AtomicPointer<std::map<int, int>> *atomicPointer) {
    while (true) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        boost::shared_ptr<std::map<int, int> > ptr = atomicPointer->get();
        for (int i = 0; i < 20; i++) {
            std::cout << i << ":" << (*ptr)[i] << " ";
        }
        std::cout << std::endl;
        std::cout.flush();
    }
};

void atomicPointerTest(){
    hazelcast::util::AtomicPointer<std::map<int, int>> atomicPointer;
    boost::thread ft(putMap, &atomicPointer);
    boost::thread f2(putMap, &atomicPointer);
    boost::thread st(printMap, &atomicPointer);
    boost::thread s1(printMap, &atomicPointer);
    boost::thread s2(printMap, &atomicPointer);
    boost::thread s3(printMap, &atomicPointer);

    ft.join();
}

int deneme::init() {
    atomicPointerTest();
    return 0;
}
