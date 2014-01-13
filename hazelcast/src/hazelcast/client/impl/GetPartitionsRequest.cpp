//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/GetPartitionsRequest.h"
#include <iostream>

namespace hazelcast {
    namespace client {
        namespace impl {
            GetPartitionsRequest::GetPartitionsRequest() {

            };

            int GetPartitionsRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;

            }

            int GetPartitionsRequest::getClassId() const {
                return protocol::ProtocolConstants::GET_PARTITIONS;

            }

            void GetPartitionsRequest::write(serialization::PortableWriter &writer) const {
            }
        }
    }
}