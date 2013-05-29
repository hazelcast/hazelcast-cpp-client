//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Member.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Member::Member():address("", 0) { //TODO

            };

            Address Member::getAddress() const {
                return address;
            };

            std::string Member::getUuid() const {
                return uuid;
            }
        }
    }
}
