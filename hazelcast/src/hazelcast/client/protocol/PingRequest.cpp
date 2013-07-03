//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "PingRequest.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            PingRequest::PingRequest() {

            }

            int PingRequest::getFactoryId() const {
                return ProtocolConstants::DATA_FACTORY_ID;
            }

            int PingRequest::getClassId() const {
                return ProtocolConstants::PING;
            }

            void PingRequest::writeData(serialization::BufferedDataOutput & writer) {

            }

            void PingRequest::readData(serialization::BufferedDataInput & reader) {

            }

        }
    }
}