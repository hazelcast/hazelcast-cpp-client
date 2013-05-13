//
// Created by sancar koyunlu on 5/12/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "TestMobile.h"


TestMobile::TestMobile() {

}

TestMobile::TestMobile(int i, char c):i(i), c(c) {

}

bool TestMobile::operator ==(TestMobile & rhs) {
    if (this == &rhs)
        return true;
    if (i != rhs.i) return false;
    if (c != rhs.c) return false;
    return true;
};


