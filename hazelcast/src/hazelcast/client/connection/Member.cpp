//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/Member.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"


namespace hazelcast {
    namespace client {
        namespace connection {
            Member::Member():address("", 0) {

            };

            Member::Member(Member const &rhs)
            :address(rhs.address)
            , uuid(rhs.uuid) {

            };


            Member::Member(Address const &rhs)
            : address(rhs) {

            };

            Member::~Member() {

            };

            Member &Member::operator = (Member const &rhs) {
                address = rhs.address;
                uuid = rhs.uuid;
                return (*this);
            };

            bool Member::operator ==(const Member &rhs) const {
                return address == rhs.address;
            };

            int Member::operator <(const Member &rhs) const {
                return address < rhs.address;
            };

            const Address &Member::getAddress() const {
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

            const std::string &Member::getAttribute(const std::string &key) const {
                return attributes.at(key);
            }

            void Member::writeData(serialization::ObjectDataOutput &writer) const {
                address.writeData(writer);
                writer.writeUTF(uuid);
                writer.writeInt(attributes.size());
                std::map<std::string, std::string>::const_iterator it;
                for (it = attributes.begin(); it != attributes.end(); ++it) {
                    writer.writeUTF(it->first);
                    writer.writeUTF(it->second);
                }
            };

            void Member::readData(serialization::ObjectDataInput &reader) {
                address.readData(reader);
                uuid = reader.readUTF();
                int size = reader.readInt();
                for (int i = 0; i < size; i++) {
                    attributes[reader.readUTF()] = reader.readUTF();
                }
            };


        }
    }
}
