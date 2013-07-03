//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "GetPartitionsRequest.h"
#include "BufferedDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            GetPartitionsRequest::GetPartitionsRequest() {

            };

            int GetPartitionsRequest::getFactoryId() const {
                return protocol::ProtocolConstants::PARTITION_DS_FACTORY;

            }

            int GetPartitionsRequest::getClassId() const {
                return protocol::ProtocolConstants::GET_PARTITIONS;

            }

            void GetPartitionsRequest::writeData(serialization::BufferedDataOutput & writer) const{

            }

            void GetPartitionsRequest::readData(serialization::BufferedDataInput & reader) {

            }


        }
    }
}