//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PartitionsResponse.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            PartitionsResponse::PartitionsResponse() {

            };

            PartitionsResponse::PartitionsResponse(std::vector<Address> & addresses, std::vector<int> & ownerIndexes)
            : members(addresses)
            , ownerIndexes(ownerIndexes) {

            };

            std::vector<Address>  PartitionsResponse::getMembers() const {
                return members;
            };

            std::vector<int>  PartitionsResponse::getOwnerIndexes() const {
                return ownerIndexes;
            };

            bool PartitionsResponse::isEmpty() {
                return members.size() == 0;
            }


        }
    }
}