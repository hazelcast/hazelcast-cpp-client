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
            PartitionsResponse::PartitionsResponse() {

            };

            PartitionsResponse::PartitionsResponse(std::vector<Address> & addresses, std::vector<int> & ownerIndexes)
            : members(addresses)
            , ownerIndexes(ownerIndexes) {

            };

            const std::vector<Address>&  PartitionsResponse::getMembers() const {
                return members;
            };

            const std::vector<int>&  PartitionsResponse::getOwnerIndexes() const {
                return ownerIndexes;
            };

            bool PartitionsResponse::isEmpty() {
                return members.size() == 0;
            };

            int PartitionsResponse::getFactoryId() const {
                return protocol::ProtocolConstants::PARTITION_DS_FACTORY;
            };

            int PartitionsResponse::getClassId() const {
                return protocol::ProtocolConstants::PARTITIONS;
            };

            void PartitionsResponse::writeData(serialization::ObjectDataOutput& writer) const {
                writer.writeInt(members.size());
                for (int i = 0; i < members.size(); i++) {
                    members[i].writeData(writer);
                }
                writer.writeInt(ownerIndexes.size());
                for (int i = 0; i < ownerIndexes.size(); i++) {
                    writer.writeInt(ownerIndexes[i]);
                }
            };

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
            };


        }
    }
}