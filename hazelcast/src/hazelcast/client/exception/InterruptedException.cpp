//
// Created by sancar koyunlu on 7/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "InterruptedException.h"


namespace hazelcast {
    namespace client {
        namespace exception {
            InterruptedException::InterruptedException(const std::string& source, const std::string& message)
            : IException(source, message) {

            }

            InterruptedException::~InterruptedException() {

            }

            char const *InterruptedException::what() const {
                return IException::what();
            }
        }

    }
}
