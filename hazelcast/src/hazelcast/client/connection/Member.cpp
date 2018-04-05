/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/29/13.

#include "hazelcast/client/Member.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"

namespace hazelcast {
    namespace client {
        Member::Member() {
        }

        Member::Member(const Address &address, const std::string &uuid, bool lite,
                       const std::map<std::string, std::string> &attr) :
                address(address), uuid(uuid), liteMember(lite), attributes(attr) {
        }

        Member::Member(const Address &memberAddress) : address(memberAddress) {
        }

        bool Member::operator==(const Member &rhs) const {
            return address == rhs.address;
        }

        const Address &Member::getAddress() const {
            return address;
        }

        const std::string &Member::getUuid() const {
            return uuid;
        }

        bool Member::isLiteMember() const {
            return liteMember;
        }

        const std::map<std::string, std::string> &Member::getAttributes() const {
            return attributes;
        }

        std::ostream &operator<<(std::ostream &out, const Member &member) {
            const Address &address = member.getAddress();
            out << "Member[";
            out << address.getHost();
            out << "]";
            out << ":";
            out << address.getPort();
            out << " - " << member.getUuid();
            return out;
        }

        const std::string *Member::getAttribute(const std::string &key) const {
            std::map<std::string, std::string>::const_iterator it = attributes.find(key);
            if (attributes.end() != it) {
                return &(it->second);
            } else {
                return NULL;
            }
        }

        bool Member::lookupAttribute(const std::string &key) const {
            return attributes.find(key) != attributes.end();
        }

        void Member::setAttribute(const std::string &key, const std::string &value) {
            attributes[key] = value;
        }

        bool Member::removeAttribute(const std::string &key) {
            return 0 != attributes.erase(key);
        }

        bool Member::operator<(const Member &rhs) const {
            return uuid < rhs.uuid;
        }

        void Member::updateAttribute(Member::MemberAttributeOperationType operationType, const std::string &key,
                                     std::auto_ptr<std::string> &value) {
            switch (operationType) {
                case PUT:
                    attributes[key] = *value;
                    break;
                case REMOVE:
                    attributes.erase(key);
                    break;
                default:
                    throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Member::updateAttribute")
                            << "Not a known OperationType: " << operationType).build();
            }
        }
    }
}

