//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AtomicInteger.h"

namespace hazelcast {
    namespace util {
        AtomicInteger::AtomicInteger():integer(0) {

        };

        AtomicInteger::AtomicInteger(int i):integer(i) {

        };

        AtomicInteger::AtomicInteger(AtomicInteger const & rhs) {
            __sync_lock_test_and_set(&integer, rhs.integer);
        };


        AtomicInteger  & AtomicInteger::operator = (AtomicInteger const & rhs) {
            __sync_lock_test_and_set(&integer, rhs.integer);
            return (*this);
        };

        AtomicInteger  & AtomicInteger::operator = (int i) {
            __sync_lock_test_and_set(&integer, i);
            return (*this);
        };


        AtomicInteger & AtomicInteger::operator ++() {
            __sync_fetch_and_add(&integer, 1);
            return (*this);
        };

        AtomicInteger AtomicInteger::operator ++(int) {
            AtomicInteger tmp(*this);
            operator++();
            return tmp;
        }

        int AtomicInteger::getAndAdd(int i) {
            return __sync_fetch_and_add(&integer, i);
        };

        int AtomicInteger::get() const {
            return __sync_fetch_and_add(&integer, 0);
        };

        void AtomicInteger::set(int i) {
            __sync_lock_test_and_set(&integer, i);
        };


    }
}