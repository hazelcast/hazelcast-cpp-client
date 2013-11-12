//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "NoSuchElementException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            NoSuchElementException::NoSuchElementException(const std::string &source, const std::string &message)
            : IException(source, message) {

            }

            NoSuchElementException::~NoSuchElementException()  throw() {

            }

            char const *NoSuchElementException::what() const throw() {
                return IException::what();
            }
        }
    }
}