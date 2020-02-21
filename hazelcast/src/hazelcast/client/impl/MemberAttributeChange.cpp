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
// Created by sancar koyunlu on 23/01/14.
//

#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/impl/MemberAttributeChange.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            MemberAttributeChange::MemberAttributeChange() {

            }

            MemberAttributeChange::MemberAttributeChange(std::unique_ptr<std::string> &uuid,
                                                         MemberAttributeEvent::MemberAttributeOperationType const &operationType,
                                                         std::unique_ptr<std::string> &key, std::unique_ptr<std::string> &value)
                    : uuid(std::move(uuid)),
                      operationType(operationType),
                      key(std::move(key)),
                      value(std::move(value)) {
            }

            const std::string &MemberAttributeChange::getUuid() const {
                return *uuid;
            }

            MemberAttributeEvent::MemberAttributeOperationType MemberAttributeChange::getOperationType() const {
                return operationType;
            }

            const std::string &MemberAttributeChange::getKey() const {
                return *key;
            }

            const std::string &MemberAttributeChange::getValue() const {
                return *value;
            }
        }
    }
}
