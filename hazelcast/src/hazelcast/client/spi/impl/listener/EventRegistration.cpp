/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on 12/11/15.

#include "hazelcast/client/spi/impl/listener/EventRegistration.h"
#include "hazelcast/client/protocol/codec/IAddListenerCodec.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    EventRegistration::EventRegistration(int64_t callId, const Address &member,
                                                         std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec)
                            : correlationId(callId), memberAddress(member), addCodec(addCodec) {
                    }

                    int64_t EventRegistration::getCorrelationId() const {
                        return correlationId;
                    }

                    const Address &EventRegistration::getMemberAddress() const {
                        return memberAddress;
                    }

                    const protocol::codec::IAddListenerCodec *EventRegistration::getAddCodec() const {
                        return addCodec.get();
                    }

                    void EventRegistration::setCorrelationId(int64_t callId) {
                        correlationId = callId;
                    }
                }
            }
        }

    }
}
