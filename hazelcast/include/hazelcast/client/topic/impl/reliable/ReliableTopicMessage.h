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
// Created by ihsan demir on 27 May 2016.

#ifndef HAZELCAST_CLIENT_TOPIC_IMPL_RELIABLE_RELIABLETOPICMESSAGE_H_
#define HAZELCAST_CLIENT_TOPIC_IMPL_RELIABLE_RELIABLETOPICMESSAGE_H_

#include <memory>

#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    class HAZELCAST_API ReliableTopicMessage : public serialization::IdentifiedDataSerializable {
                    public:
                        ReliableTopicMessage();

                        ReliableTopicMessage(serialization::pimpl::Data payloadData, std::auto_ptr<Address> address);

                        int64_t getPublishTime() const;

                        const Address *getPublisherAddress() const;

                        const serialization::pimpl::Data &getPayload() const;

                        virtual int getFactoryId() const;

                        virtual int getClassId() const;

                        virtual void writeData(serialization::ObjectDataOutput &writer) const;

                        virtual void readData(serialization::ObjectDataInput &reader);
                    private:
                        int64_t publishTime;
                        std::auto_ptr<Address> publisherAddress;
                        serialization::pimpl::Data payload;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_TOPIC_IMPL_RELIABLE_RELIABLETOPICMESSAGE_H_

