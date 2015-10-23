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
// Created by sancar koyunlu on 6/4/13.



#include "hazelcast/client/impl/GetPartitionsRequest.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            GetPartitionsRequest::GetPartitionsRequest() {

            }

            int GetPartitionsRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;

            }

            int GetPartitionsRequest::getClassId() const {
                return protocol::ProtocolConstants::GET_PARTITIONS;
            }


            bool GetPartitionsRequest::isRetryable() const {
                return true;
            }

            void GetPartitionsRequest::write(serialization::PortableWriter &writer) const {
            }
        }
    }
}
