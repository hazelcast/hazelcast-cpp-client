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
// Created by sancar koyunlu on 8/5/13.



#include "hazelcast/client/collection/TxnSetAddRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnSetAddRequest::TxnSetAddRequest(const std::string& name, const serialization::pimpl::Data& data)
            : TxnCollectionRequest(name, data) {

            }

            int TxnSetAddRequest::getClassId() const {
                return CollectionPortableHook::TXN_SET_ADD;
            }

        }
    }
}
