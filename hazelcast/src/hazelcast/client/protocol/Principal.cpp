/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 5/20/13.

#include "hazelcast/client/protocol/Principal.h"

namespace hazelcast {
    namespace client {
        namespace protocol {

            Principal::Principal(std::unique_ptr<std::string> &id, std::unique_ptr<std::string> &owner) : uuid(std::move(id)),
                                                                                                    ownerUuid(std::move(owner)) {
            }

            const std::string *Principal::getUuid() const {
                return uuid.get();
            }

            const std::string *Principal::getOwnerUuid() const {
                return ownerUuid.get();
            }

            bool Principal::operator==(const Principal &rhs) const {
                if (ownerUuid.get() != NULL ? (rhs.ownerUuid.get() == NULL || *ownerUuid != *rhs.ownerUuid) :
                    ownerUuid.get() != NULL) {
                    return false;
                }

                if (uuid.get() != NULL ? (rhs.uuid.get() == NULL || *uuid != *rhs.uuid) : rhs.uuid.get() != NULL) {
                    return false;
                }

                return true;
            }

            std::ostream &operator<<(std::ostream &os, const Principal &principal) {
                os << "uuid: " << (principal.uuid.get() ? *principal.uuid : "null") << " ownerUuid: "
                   << (principal.ownerUuid ? *principal.ownerUuid : "null");
                return os;
            }
        }
    }
}
