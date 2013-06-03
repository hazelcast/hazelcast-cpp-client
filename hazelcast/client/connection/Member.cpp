//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Member.h"


namespace hazelcast {
    namespace client {
        namespace connection {
            Member::Member():address("", 0) { //TODO

            };

            Member::Member(Member const & rhs)
            :address(rhs.address)
            , uuid(rhs.uuid) {

            };

            Member::~Member() {

            };

            Member & Member::operator = (Member const & rhs) {
                address = rhs.address;
                uuid = rhs.uuid;
                return (*this);
            };

            bool Member::operator ==(const Member & rhs) const {
                return address == rhs.address;
            };

            int Member::operator <(const Member& rhs) const {
                return address < rhs.address;
            };

            Address Member::getAddress() const {
                return address;
            };

            std::string Member::getUuid() const {
                return uuid;
            };
        }
    }
}
