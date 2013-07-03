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

            int Principal::getFactoryId() const {
                return ProtocolConstants::CLIENT_PORTABLE_FACTORY;;
            }

            int Principal::getClassId() const {
                return ProtocolConstants::PRINCIPAL_ID;;
            }


        }
    }
}