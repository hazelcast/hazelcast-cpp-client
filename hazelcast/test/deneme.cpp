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


template <typename  T, typename V>
static V getValueFromServer(T x) {
    V v;
    return v;
};

template <typename Key, typename Value>
class Map {
public:
    class value_reference;

    value_reference operator [](const Key& k) {
        return value_reference(k);
    };

    //can be outside of the Map class, carrying the key as bytes
    class value_reference {
    public:
        value_reference(const Key& k):k(k) {

        };

        operator Value () {

            return getValueFromServer<Key, Value>(k);
        };


        Key k;
    private:

    };

private:
};

void callable (){
        while (true) {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
            std::cout << "zZ " ;
            std::cout.flush();
        }
};

void start(){
    boost::thread s(callable);
}

int deneme::init() {
    Map<int, int> m;
    Map<int, int>::value_reference reference = m[4];
    int a = reference;
    std::cout << a << std::endl;

    return 0;
}
