//
// Created by sancar koyunlu on 5/20/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Principal.h"


namespace hazelcast {
    namespace client {
        namespace protocol {


            Principal::Principal() {

            }

            Principal::Principal(std::string uuid, std::string ownerUuid)
            : uuid(uuid)
            , ownerUuid(ownerUuid) {

            }


        }
    }
}