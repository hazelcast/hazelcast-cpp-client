//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Member.h"
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"


namespace hazelcast {
    namespace client {
        namespace connection {
            Member::Member():address("", 0) {

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

            const Address& Member::getAddress() const {
                return address;
            };

            std::string Member::getUuid() const {
                return uuid;
            };

            int Member::getFactoryId() const {
                return protocol::ProtocolConstants::DATA_FACTORY_ID;
            };

            int Member::getClassId() const {
                return protocol::ProtocolConstants::MEMBER_ID;
            };

            void Member::writeData(serialization::BufferedDataOutput & writer) {
                address.writeData(writer);
                writer.writeUTF(uuid);
            };

            void Member::readData(serialization::BufferedDataInput & reader) {
                address.readData(reader);
                uuid = reader.readUTF();
            };


        }
    }
}
