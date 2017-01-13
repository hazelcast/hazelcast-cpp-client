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
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"
#include "hazelcast/client/topic/impl/TopicDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicMessage::ReliableTopicMessage() {
                    }

                    ReliableTopicMessage::ReliableTopicMessage(
                            hazelcast::client::serialization::pimpl::Data payloadData, std::auto_ptr<Address> address)
                            : publishTime(util::currentTimeMillis()), publisherAddress(address), payload(payloadData) {
                    }

                    int64_t ReliableTopicMessage::getPublishTime() const {
                        return publishTime;
                    }

                    const Address *ReliableTopicMessage::getPublisherAddress() const {
                        return publisherAddress.get();
                    }

                    const serialization::pimpl::Data &ReliableTopicMessage::getPayload() const {
                        return payload;
                    }

                    int ReliableTopicMessage::getFactoryId() const {
                        return F_ID;
                    }

                    int ReliableTopicMessage::getClassId() const {
                        return RELIABLE_TOPIC_MESSAGE;
                    }

                    void ReliableTopicMessage::writeData(serialization::ObjectDataOutput &out) const {
                        out.writeLong(publishTime);
                        out.writeObject<serialization::IdentifiedDataSerializable>(publisherAddress.get());
                        out.writeData(&payload);
                    }

                    void ReliableTopicMessage::readData(serialization::ObjectDataInput &in) {
                        publishTime = in.readLong();
                        publisherAddress = in.readObject<Address>();
                        payload = in.readData();
                    }
                }
            }
        }
    }
}

