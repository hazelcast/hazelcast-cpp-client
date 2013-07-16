//
// Created by sancar koyunlu on 7/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include <ostream>
#include <iostream>
#include "deneme.h"


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


int deneme::init() {
    Map<int,int> m;
    Map<int,int>::value_reference reference = m[4];
    int a = reference;
    std::cout << a << std::endl;

}
