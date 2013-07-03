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

            int HazelcastServerError::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int HazelcastServerError::getClassId() const {
                return protocol::ProtocolConstants::HAZELCAST_SERVER_ERROR_ID;
            }

        }
    }
}
