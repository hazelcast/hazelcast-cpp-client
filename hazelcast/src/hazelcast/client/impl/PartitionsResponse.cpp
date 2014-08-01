//
// Created by sancar koyunlu on 6/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PartitionsResponse.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"


namespace hazelcast {
    namespace client {
        namespace impl {

            const std::vector<Address>&  PartitionsResponse::getMembers() const {
                return members;
            }

            const std::vector<int>&  PartitionsResponse::getOwnerIndexes() const {
                return ownerIndexes;
            }

            int PartitionsResponse::getFactoryId() const {
                return protocol::ProtocolConstants::PARTITION_DS_FACTORY;
            }

            int PartitionsResponse::getClassId() const {
                return protocol::ProtocolConstants::PARTITIONS;
            }

            void PartitionsResponse::readData(serialization::ObjectDataInput& reader) {
                int len;
                len = reader.readInt();
                members.resize(len);
                for (int i = 0; i < len; i++) {
                    members[i].readData(reader);
                }
                len = reader.readInt();
                ownerIndexes.resize(len);
                for (int i = 0; i < len; i++) {
                    ownerIndexes[i] = reader.readInt();
                }
            }


        }
    }
}
