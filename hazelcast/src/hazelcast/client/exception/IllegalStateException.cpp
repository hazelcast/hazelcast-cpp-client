//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "IllegalStateException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            IllegalStateException::IllegalStateException(const std::string& source, const std::string& message)
            : IException(source, message) {

            }

            IllegalStateException::~IllegalStateException() {

            }

            char const *IllegalStateException::what() const {
                return IException::what();
            }
        }

    }
}