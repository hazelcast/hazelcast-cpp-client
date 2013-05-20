//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "HazelcastServerError.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            HazelcastServerError::HazelcastServerError() {

            }

            HazelcastServerError::~HazelcastServerError() {
            }

            char const *HazelcastServerError::what() const {
                std::string error = message + ":" + details;
                return error.c_str();
            }

        }
    }
}
