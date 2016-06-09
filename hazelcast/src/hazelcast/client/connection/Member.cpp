/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

        std::ostream &operator<<(std::ostream &stream, const Member &member) {
            return stream << "Member[" << member.getAddress() << "]";
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
    }
}

